#pragma once
#include <list>
#include <atomic>
#include <mutex>
#include <cstdlib>
#include <algorithm>
#include "tools/singleton.h"
#include "debug_print.h"
#include "queuelockfree/concurrentqueue.h"

namespace ToolBox
{

#define ENABLE_OBJECT_POOL_LOCK_FREE 0

#ifndef ENABLE_OBJECT_POOL_LOCK_FREE
#define ENABLE_OBJECT_POOL_LOCK_FREE 1
#endif

#ifndef ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE
#define ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE 0
#endif


    /*
    * 定义对象池
    */
    template<typename ObjectType, std::size_t Count = 128>
    class ObjectPoolLockFree : public DebugPrint
    {
    public:
        ObjectPoolLockFree(const ObjectPoolLockFree&) = delete;
        ObjectPoolLockFree(ObjectPoolLockFree&&) = delete;
        ObjectPoolLockFree<ObjectType>& operator=(const ObjectType&) = delete;

        /*
        * 构造
        */
        ObjectPoolLockFree()
        // : allocated_count_(0)
        {
#if ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE
            SetDebugPrint(true);
#endif // ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE
            Expand();
        };
        /*
        * 析构
        */
        ~ObjectPoolLockFree()
        {
        }
        /*
        * 获取对象
        */
        template<typename...Args>
        ObjectType* GetObjectLockFree(Args...args)
        {
#if ENABLE_OBJECT_POOL_LOCK_FREE
            ObjectType* mem = nullptr;
            // allocated_count_.fetch_add(1);
            {
                std::lock_guard<std::mutex> lock(lock_);
                if (0 == free_objects_.size_approx())
                {
                    Expand();
                }
                free_objects_.try_dequeue(mem);
            }
            auto* object = new (mem)ObjectType(args...);
            return object;
#else
            return new ObjectType(args...);
#endif
        }
        /*
        * 回收对象
        */
        void GiveBackLockFree(ObjectType* object, std::string debug_tag = "")
        {
#if ENABLE_OBJECT_POOL_LOCK_FREE
            // allocated_count_.fetch_sub(1);
            object->~ObjectType();
#if ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE
            auto iter = find(free_objects_.begin(), free_objects_.end(), object);
            if (iter != free_objects_.end())
            {
                Print("[对象池] debug_tag:%s 重复回收对象:%p\n", debug_tag.c_str(), object);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 1000));
                return;
            }
#endif // ENABLE_DEBUG_OBJECT_POOL_LOCK_FREE
            free_objects_.enqueue(object);
#else
            delete object;
            object = nullptr;
#endif
        }

        /*
        * 测试打印
        */
        void DebugPrint()
        {
            if (!GetDebugStatus())
            {
                return;
            }
            Print("\n ========== ObjectPool ========== \n");
            Print("FreeObjects 个数:%zu,using_memory_ 个数:%zu\n", free_objects_.size_approx(), using_memory_.size_approx());
            Print("\n ---------- ObjectPool ---------- \n");
        }
    private:
        /*
        * 扩容
        */
        void Expand()
        {
            std::unique_ptr<char> memory = std::unique_ptr<char>(new char[Count * sizeof(ObjectType)]);
            for (std::size_t i = 0; i < Count; i++)
            {
                free_objects_.enqueue(reinterpret_cast<ObjectType*>(memory.get() + i * sizeof(ObjectType)));
            }
            using_memory_.enqueue(std::move(memory));
        }

    private:
        using FreeObjects = moodycamel::ConcurrentQueue<ObjectType*>;
        using UsingMemory = moodycamel::ConcurrentQueue<std::unique_ptr<char>>;
        FreeObjects free_objects_;  // 可用对象
        UsingMemory using_memory_;  // 正在使用的内存
        // std::atomic<size_t> allocated_count_;   // 已分配对象的数量
        std::mutex lock_;
    };

    /*
    * 对象池单例
    */
    template<typename ObjectType>
    static ObjectPoolLockFree<ObjectType>& GetObjectPoolLockFreeMgrRef()
    {
        return *(Singleton<ObjectPoolLockFree<ObjectType>>::Instance());
    }

    /*
    * 从对象池单例中构造一个对象
    * @param ObjectType 对象类型
    * @param Args 参数
    */
    template <typename ObjectType, typename...Args>
    static ObjectType* GetObjectLockFree(Args...args)
    {
        return GetObjectPoolLockFreeMgrRef<ObjectType>().GetObjectLockFree(args...);
    }

    /*
    * 归还对象
    */
    template<typename ObjectType>
    static void GiveBackObjectLockFree(ObjectType* object, std::string debug_tag = "")
    {
        GetObjectPoolLockFreeMgrRef<ObjectType>().GiveBackLockFree(object, debug_tag);
    }

};  // ToolBox