#pragma once
#include <list>
#include <atomic>
#include <mutex>
#include <cstdlib>
#include <algorithm>
#include "src/tools/singleton.h"
#include "debug_print.h"

namespace ToolBox{

#define ENABLE_DEBUG_OBJECT_POOL 0

/*
* 定义对象池
*/
template<typename ObjectType, std::size_t Count = 128>
class ObjectPool : public DebugPrint
{
public:
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool<ObjectType>& operator=(const ObjectType&) = delete;

    /*
    * 构造
    */
    ObjectPool()
    : allocated_count_(0)
    {
#if ENABLE_DEBUG_OBJECT_POOL
        SetDebugPrint(true);
#endif // ENABLE_DEBUG_OBJECT_POOL
        Expand();
    };
    /*
    * 析构
    */
    ~ObjectPool()
    {
        free_objects_.clear();
        using_memory_.clear();
    }
    /*
    * 获取对象
    */
    template<typename...Args>
    ObjectType* GetObject(Args...args)
    {
        ObjectType* mem = nullptr;
        allocated_count_.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(lock_);
            if(free_objects_.empty())
            {
                Expand();
            }
            mem = free_objects_.front();
            free_objects_.pop_front();
        }
        auto* object = new(mem)ObjectType(args...);
        return object;
    }
    /*
    * 回收对象
    */
   void GiveBack(ObjectType* object, std::string debug_tag = "")
   {
       allocated_count_.fetch_sub(1);
       object->~ObjectType();
       std::lock_guard<std::mutex> lock(lock_);
#if ENABLE_DEBUG_OBJECT_POOL
       auto iter = find(free_objects_.begin(), free_objects_.end(), object);
       if(iter != free_objects_.end())
       {
           Print("[对象池] debug_tag:%s 重复回收对象:%p\n", debug_tag.c_str(), object);
           std::this_thread::sleep_for(std::chrono::milliseconds(1000*1000));
           return;
       }
#endif // ENABLE_DEBUG_OBJECT_POOL
       free_objects_.emplace_back(object);
   }

    /*
    * 测试打印
    */
    void DebugPrint()
    {
        if(!GetDebugStatus())
        {
            return;
        }
        Print("\n ========== ObjectPool ========== \n");
        Print("FreeObjects 个数:%zu,using_memory_ 个数:%zu\n", free_objects_.size(), using_memory_.size());
        Print("\n ---------- ObjectPool ---------- \n");
    }
private:
    /*
    * 扩容
    */
    void Expand()
    {
        std::unique_ptr<char> memory = std::unique_ptr<char>(new char[Count * sizeof(ObjectType)]);
        for(std::size_t i = 0; i < Count; i++)
        {
            free_objects_.emplace_back(reinterpret_cast<ObjectType*>(memory.get() + i * sizeof(ObjectType)));
        }
        using_memory_.emplace_back(std::move(memory));
    }
    
private:
    using FreeObjects = std::list<ObjectType*>;
    using UsingMemory = std::list<std::unique_ptr<char>>;
    FreeObjects free_objects_;  // 可用对象
    UsingMemory using_memory_;  // 正在使用的内存
    std::atomic<size_t> allocated_count_;   // 已分配对象的数量
    std::mutex lock_;
};

/*
* 对象池单例
*/
template<typename ObjectType>
static ObjectPool<ObjectType>& GetObjectPoolMgrRef()
{
    return *(Singleton<ObjectPool<ObjectType>>::Instance());
}

/*
* 从对象池单例中构造一个对象
* @param ObjectType 对象类型
* @param Args 参数
*/
template <typename ObjectType, typename...Args>
static ObjectType* GetObject(Args...args)
{
    return GetObjectPoolMgrRef<ObjectType>().GetObject(args...);
}

/*
* 归还对象
*/
template<typename ObjectType>
static void GiveBackObject(ObjectType* object, std::string debug_tag = "")
{
    GetObjectPoolMgrRef<ObjectType>().GiveBack(object, debug_tag);
}

};  // ToolBox