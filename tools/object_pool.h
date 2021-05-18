#pragma once
#include <list>
#include <atomic>
#include <mutex>
#include <cstdlib>
/*
* 定义对象池
*/
template<typename ObjectType, size_t Count = 128>
class ObjectPool
{
public:
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool<ObjectType>& operator=(const ObjectType&) = delete;

    /*
    * 构造
    */
    ObjectPool()
    {
        Expand();
    };
    /*
    * 析构
    */
    ~ObjectPool()
    {
        for(auto mem : using_memory_)
        {
            delete[] mem;
        }
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
   void GiveBack(ObjectType* object)
   {
       allocated_count_.fetch_sub(1);
       object->~ObjectType();
       std::lock_guard<std::mutex> lock(lock_);
       free_objects_.push_back(object);
   }
private:
    /*
    * 扩容
    */
    void Expand()
    {
        auto* memory = new char[Count * sizeof(ObjectType)];
        for(auto i = 0; i < Count; i++)
        {
            free_objects_.push_back(reinterpret_cast<ObjectType*>(memory + i * sizeof(ObjectType)));
        }
        using_memory_.push_back(memory);
    }
    
private:
    using FreeObjects = std::list<ObjectType*>;
    using UsingMemory = std::list<char*>;
    FreeObjects free_objects_;  // 可用对象
    UsingMemory using_memory_;  // 正在使用的内存
    std::atomic<size_t> allocated_count_ = 0;   // 已分配对象的数量
    std::mutex lock_;
};