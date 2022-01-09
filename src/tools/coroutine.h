#pragma once

// 改编自云峰大佬的实现方式
#ifdef __linux__

#include <ucontext.h>
#include <functional>
#include <stddef.h>
#include <vector>
#include <string.h>

namespace ToolBox{

using  CoroutineFunc = std::function< void (class Schedule*, void *ud) >; 

enum class COROUTINE_STATUS
{
    COROUTINE_DEAD = 0,
    COROUTINE_READY = 1,
    COROUTINE_RUNNING = 2,
    COROUTINE_SUSPEND = 3,
};

class Schedule;
/*
* 定义协程
*/
class Coroutine
{
public:
    Coroutine(Schedule *sch, CoroutineFunc coroutine_func, void *ud)
    {
        func_ = coroutine_func;
        ud_ = ud;
        sch_ = sch;
        cap_ = 0;
        size_ = 0;
        status_ = COROUTINE_STATUS::COROUTINE_READY;
        stack_ = nullptr;
    }
    ~Coroutine(){
        delete stack_;
    }
    /*
    * 获取状态
    */
    COROUTINE_STATUS GetStatus(){ return status_; }
    /*
    *  设置状态
    */
    void SetStatus(COROUTINE_STATUS status){ status_ = status; }
    /*
    * 获取协程上下文
    */
    ucontext_t& GetUcontext(){ return ctx_; }
    /*
    * 获取运行时栈的大小
    */
    ptrdiff_t GetSize(){ return size_; }
    /*
    * 重新设置大小
    */
    void SetSize(ptrdiff_t size){ size_ = size; }
    /*
    * 获取运行时栈
    */
    char* GetStack(){ return stack_; }
    /*
    * 获取已经分配的内存大小
    */
    ptrdiff_t GetCap(){ return cap_; }
    /*
    * 设置 cap
    */
    void SetCap(ptrdiff_t cap){ cap_ = cap; }
    /*
    * 重新分配栈
    */
    void ReNewStack()
    {
        delete stack_;
        stack_ = new char[cap_];
    }
    /*
    * 执行回调函数
    */
    void InvokeFunc(Schedule* sch, void *ud)
    {
        func_(sch, ud);
    }
    /*
    * 获取协程参数
    */
    void* GetParamUd(){ return ud_;}
private:
    CoroutineFunc func_;             // 协程所用的函数
    void *ud_;                       // 协程参数
    ucontext_t ctx_;                 // 协程上下文
    Schedule *sch_;                  // 该协程所述的调度器
    ptrdiff_t cap_;                  // 已经分配的内存大小
    ptrdiff_t size_;                 // 当前协程运行时栈,保存起来后的大小
    COROUTINE_STATUS status_;        // 协程当前的状态
    char* stack_;                    // 当前协程的保存起来的运行时栈
};

/*
* 协程调度器
*/
class Schedule 
{
public:
    /*
    * 构造
    */
    Schedule()
    {
        nco_ = 0;
        running_ = running_init_;
    }
    ~Schedule()
    {
        for (size_t i = 0; i < co_vec_.size(); i++)
        {
            CoroutineDelete(i);
        }
        
    }
    /*
    * 创建协程
    */
    int CoroutineNew(CoroutineFunc func, void *ud)
    {
        auto* co = new Coroutine(this, func, ud);
        if(nco_ >= co_vec_.size())
        {
            co_vec_.emplace_back(co);
            nco_++;
            return co_vec_.size() - 1;
        } else
        {
            for(size_t i = 0; i < co_vec_.size(); i++)
            {
                size_t id = (i + nco_) % co_vec_.size();
                if(nullptr == co_vec_[id])
                {
                    co_vec_[id] = co;
                    nco_++;
                    return id;
                }
            }
        }
        return -1;
    }
    /*
    * 
    */
    static void mainfunc(uint32_t low32, uint32_t hi32)
    {
        uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
        auto* sch = (Schedule*)ptr;
        int id = sch->CoroutineRunning();
        auto* co = sch->GetCoroutine(id);
        if(nullptr == co)
        {
            return;
        }
        co->InvokeFunc(sch, co->GetParamUd());
        sch->CoroutineDelete(id);

        
    }
    /*
    * 重新开始协程
    */
    void CoroutineResume(int32_t id)
    {
        if(running_init_ != running_)
        {
            return;
        }
        auto* co = GetCoroutine(id);
        if(nullptr == co)
        {
            return;
        }
        switch (co->GetStatus())
        {
        case COROUTINE_STATUS::COROUTINE_READY:
            {
                auto& ucontext = co->GetUcontext();
                getcontext(&ucontext);
                ucontext.uc_stack.ss_sp = stack_;
                ucontext.uc_stack.ss_size = stack_size_;
                ucontext.uc_link = &main_;
                running_ = id;
                co->SetStatus(COROUTINE_STATUS::COROUTINE_RUNNING);
                uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
                makecontext(&ucontext,(void (*)(void))mainfunc, 2, static_cast<uint32_t>(ptr), static_cast<uint32_t>(ptr >> 32));
                swapcontext(&main_, &ucontext);
                break;
            }
        case COROUTINE_STATUS::COROUTINE_SUSPEND:
        {
            memmove(stack_ + stack_size_ - co->GetSize(), co->GetStack(), co->GetSize());
            running_ = id;
            co->SetStatus(COROUTINE_STATUS::COROUTINE_RUNNING);
            auto &ucontext = co->GetUcontext();
            swapcontext(&main_, &ucontext);
            break;
        }
        default:
            break;
        }
    } 
    /*
    * 让出 
    */
    void CoroutineYield()
    {
        int32_t id = running_;
        if(id < 0)
        {
            return;
        }
        auto* co = co_vec_[id];
        if((char*)&co <= stack_)
        {
            return;
        }
        SaveStack(co, stack_ + stack_size_);    // 栈的生长方向是从高地址往低地址，因此栈底的就是内存地址最大的位置，即 S->stack + STACK_SIZE 就是栈底位置。
        co->SetStatus(COROUTINE_STATUS::COROUTINE_SUSPEND);
        running_ = running_init_;
        swapcontext(&co->GetUcontext(), &main_);
    }
    /*
    * 协程状态
    */
    COROUTINE_STATUS CoroutineStatus(int32_t id)
    {
        auto* co = GetCoroutine(id);
        if(nullptr == co)
        {
            return COROUTINE_STATUS::COROUTINE_DEAD;
        }
        return co->GetStatus();
    }
    /*
    * 调度器运行状态
    */
    int32_t CoroutineRunning()
    {
        return running_;
    }
private:
    /*
    * 保存堆栈
    */
    static void SaveStack(Coroutine *co, char* top)
    {
        char dummy = 0;
        auto diff = top - &dummy;
        if(diff > (int32_t)stack_size_)
        {
            return;
        }
        if(co->GetCap() < diff )
        {
            co->SetCap(diff);
            co->ReNewStack();
        }
        co->SetSize(diff);
        memmove(co->GetStack(), &dummy, co->GetSize());
    }
    
    /*
    * 删除协程
    */
    void CoroutineDelete(int32_t id)
    {
        auto* co = GetCoroutine(id);
        if(nullptr == co)
        {
            return;
        }
        delete(co);
        co_vec_[id] = nullptr;
        nco_--;
        running_ = running_init_;
    }
    /*
    * 根据 id 获取协程
    */
    Coroutine* GetCoroutine(int32_t id)
    {
        if(id < 0 || id > (int32_t)co_vec_.size())
        {
            return nullptr;
        }
        return co_vec_[id];
    }

private:
private:
    constexpr static int32_t stack_size_ = 1024 * 1024;
    constexpr static int32_t running_init_ = -1;
    char stack_[stack_size_];         // 运行时栈，此栈即是共享栈
    ucontext_t main_;                // 主协程的上下文
    size_t nco_;                    // 当前存活的协程个数
    int32_t running_;                // 正在运行的协程ID
    std::vector<Coroutine*> co_vec_;     // 一个一维数组,用于存放所有协程。其长度等于cap
};

};  // ToolBox

#endif // __linux__