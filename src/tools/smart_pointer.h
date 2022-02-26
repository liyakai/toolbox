#pragma once
#include <stdint.h>
#include <utility>
namespace ToolBox{

template<typename T>
class SmartPtr;

/*
* 定义辅助类
* 辅助类用以封装使用计数与基础对象指针
*/
template<typename T>
class U_Ptr
{
private:
    friend class SmartPtr<T>;
    friend class WeakPtr<T>;
    /* 构造 */
    U_Ptr(T* p)
        : ptr_(p), strong_count_(1), weak_count_(0)
        {}
    ~U_Ptr()
    {
        delete ptr_;
    }

private:
    T* ptr_ = nullptr;   // 对象的指针
    std::atomic<int32_t> strong_count_ = 0; // share_ptr 引用计数
    std::atomic<int32_t> weak_count_ = 0;  // weak_ptr 的引用计数
};


/*
* 定义智能指针
*/
template<typename T>
class SmartPtr
{
public:
    /*
    * 构造函数
    */
    SmartPtr(T* ptr)
        :rp_(new U_Ptr<T>(ptr))
        {}
    /*
    * 拷贝构造函数
    */
    SmartPtr(const SmartPtr &sp)
    : rp_(sp.rp_)
    {
        rp_->strong_count_++;
    }
    /*析构*/
    ~SmartPtr()
    {
        if(--rp_->strong_count_ == 0)
        {
            if(rp_->weak_count_ == 0)
            {
                delete rp_;     // 当强引用计数器和弱引用计数器都为0时,删除辅助类,且删除对象
                rp_ = nullptr;
            } else 
            {
                delete rp_->ptr_;    // 当强引用计数为0,弱引用计数不为0的时候只删除对象,不删除辅助类对象
                rp_->ptr_ = nullptr;
            }
        }
    }
    /*
    * 赋值
    */
    SmartPtr& operator=(const SmartPtr& rhs)
    {
        rhs.rp_->strong_count++;
        if(--rp_->strong_count == 0)
        {
            if(rp_->weak_count_ == 0)
            {
                delete rp_;     // 当强引用计数器和弱引用计数器都为0时,删除服务类,且删除对象
                rp_ = nullptr;
            } else 
            {
                delete rp_->ptr_;    // 当强引用计数为0,弱引用计数不为0的时候只删除对象,不删除辅助类对象
                rp_->ptr_ = nullptr;
            }
        }
        rp_ = rhs.rp_;
        return *this;
    }
    /*
    * 重载*操作符
    */
    T& operator*()
    {
        return *(rp_->ptr_);
    }
    /*
    * 重载->操作符
    */
    T* operator->()
    {
        return rp_->ptr;
    }


private:
    U_Ptr<T>* rp_;
};

/*
* 定义 WeakPtr
*/
template<typename T>
class WeakPtr
{
public:
    /*
    * 拷贝构造函数
    */
    WeakPtr(const SmartPtr &sp)
    : rp_(sp.rp_)
    {
        rp_->weak_count_++;
    }
    /*析构*/
    ~WeakPtr()
    {
        if(--rp_->weak_count_ == 0)
        {
            if(rp_->strong_count_ == 0)
            {
                delete rp_;     // 当强引用计数器和弱引用计数器都为0时,删除辅助类,且删除对象
                rp_ = nullptr;
            }
        }
    }
    /*
    * 赋值
    */
    WeakPtr& operator=(const SmartPtr& rhs)
    {
        rhs.rp_->weak_count_++;
        if(--rp_->weak_count_ == 0)
        {
            if(rp_->strong_count_ == 0)
            {
                delete rp_;     // 当强引用计数器和弱引用计数器都为0时,删除服务类
                rp_ = nullptr;
            }
        }
        rp_ = rhs.rp_;
        return *this;
    }
private:
    U_Ptr<T>* rp_;
};

/*
* 实现 make_smart 函数模板
*/
template<typename T, typename... Args>
SmartPtr<T> make_smart(Args&&... args)
{
    return SmartPtr<T>(new T(std::forward<Args>(args)...));
}


}