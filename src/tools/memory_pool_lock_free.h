#pragma once

#include "singleton.h"
#include <list>
#include <unordered_map>
#include <algorithm>
#include "debug_print.h"
#include "queuelockfree/concurrentqueue.h"

namespace ToolBox{

#define ENABLE_MEMORY_POOL_LOCK_FREE 0

#ifndef ENABLE_MEMORY_POOL_LOCK_FREE
#define ENABLE_MEMORY_POOL_LOCK_FREE 1
#endif

#ifndef ENABLE_DEBUG_MEMORY_POOL_LOCK_FREE
#define ENABLE_DEBUG_MEMORY_POOL_LOCK_FREE 0
#endif

#define MemPoolLockFreeMgr Singleton<ToolBox::MemoryPoolLockFree>::Instance()

/*
*  相同大小内存块的管理
*/
class ChunkLockFree : public DebugPrint
{
public:
    /*
    * 构造函数
    */
    ChunkLockFree(){};
    /*
    * 析构
    */
    ~ChunkLockFree()
    {
        char* pointer = nullptr;
        while (mem_list_.try_dequeue(pointer))
        {
            delete pointer;
        }
    }
    /*
    * 设置内存块大小[只有第一次设置非零值有效]
    * @param size 内存块大小
    */
    bool SetChunkSize(std::size_t size)
    {
#if ENABLE_DEBUG_MEMORY_POOL_LOCK_FREE
        SetDebugPrint(true);
#endif // ENABLE_DEBUG_MEMORY_POOL_LOCK_FREE
        if (0 == chunk_size_)
        {
            chunk_size_ = size;
            return true;
        }
        return false;
    }
    /*
    * 获取内存块
    */
    char* GetMemory()
    {
        char* pointer = nullptr;
        if(mem_list_.try_dequeue(pointer))
        {
            return pointer;
        }
        char* new_char = new char[chunk_size_];
        return new_char;
    }
    /*
    * 归还内存块
    */
    void GiveBack(char* pointer, std::string debug_tag = "")
    {
        mem_list_.enqueue(pointer);
    }
    /*
    * 剩余内存块个数
    */
    std::size_t Size()
    {
        return mem_list_.size_approx();
    }

private:
    std::size_t chunk_size_ = 0;
    std::mutex mutex_;
    moodycamel::ConcurrentQueue<char*> mem_list_;
};

/*
* 内存池
*/
class MemoryPoolLockFree : public DebugPrint
{
public:
    /*
    * 构造函数
    */
    MemoryPoolLockFree(){};
    /*
    * 析构
    */
    ~MemoryPoolLockFree()
    {
        pool_.clear();
    }
    /*
	* 申请内存
	* @param 申请内存的大小
	*/
    char* GetMemory(std::size_t size)
    {
        size = RebuildNum(size + sizeof(std::uint32_t));
#if !ENABLE_MEMORY_POOL_LOCK_FREE
        return new char[size];
#endif // !ENABLE_MEMORY_POOL_LOCK_FREE
        auto iter = pool_.find(size);
        if (iter == pool_.end())
        {
            pool_[size].SetChunkSize(size);
        }
        auto p = reinterpret_cast<std::uint32_t*>(pool_[size].GetMemory());
        if(nullptr == p)
        {
            return nullptr;
        }
        *p = size;
        return reinterpret_cast<char*>(p) + sizeof(std::uint32_t);
    }
    /*
    * 归还内存
    * @param 归还内存的指针
    */
    void GiveBack(char* pointer, std::string debug_tag = "")
    {
#if !ENABLE_MEMORY_POOL_LOCK_FREE
        delete pointer;
        return;
#endif  // !ENABLE_MEMORY_POOL_LOCK_FREE
        if(nullptr == pointer)
        {
            return;
        }
        auto index = *reinterpret_cast<std::uint32_t*>(pointer - sizeof(std::uint32_t));
        if(!index)
        {
            delete (pointer - sizeof(std::uint32_t));
            return;
        }
        return pool_[index].GiveBack(pointer - sizeof(std::uint32_t), debug_tag);
    }
	/*
	* 调试打印
	*/
	void DebugPrint()
    {
        if (GetDebugStatus())
        {
            Print("内存池中有 %zu 种块.\n", pool_.size());
            for (auto iter = pool_.begin(); iter != pool_.end(); iter++)
            {
                Print("    内存池中大小为 %zu 的内存块还有 %zu 个.\n", iter->first, iter->second.Size());
            }
        }
    }
private:
    /*
    * 求大于等于(小于等于)一个整数最小2次幂算法
    * [算法原理见] https://blog.csdn.net/Kakarotto_/article/details/108958843
    */
    int32_t RebuildNum(int32_t num)
    {
        int32_t n = num - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n < 0 ? 1 : n + 1;
    }
private:
    std::unordered_map<std::size_t, ChunkLockFree> pool_;
};

};  // ToolBox