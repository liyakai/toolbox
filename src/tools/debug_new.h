#pragma once
#include <new>
#include <mutex>
#include <unordered_map>
#include <map>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#define ENABLE_CHECK_LEAKS 1

/*
* @file brief 接管系统的new和delete,统计内存使用情况.
*/

#ifdef ENABLE_CHECK_LEAKS

// 定义文件名长度
#ifndef DEBUG_NEW_FILENAME_LEN
#define DEBUG_NEW_FILENAME_LEN 128
#endif // DEBUG_NEW_FILENAME_LEN


// 统计打印输出内存使用情况
#define CHECK_LEAKS CheckLeaks();


/*
* @brief 定义存储内存信息的类
*/
class RecordMemory
{
public:
    /*
    * @brief 增加内存申请记录
    */
    void AddNewAddr(void* pointer, std::size_t size, const char* file, int32_t line)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        new_addr_map_.insert(std::make_pair(reinterpret_cast<uint64_t>(pointer), NewAddr(file,line,size)));
    }

    /*
    * @brief 消除内存申请记录
    */
    bool RemoveNewAddr(void* pointer)
    {
        uint64_t point_v = reinterpret_cast<uint64_t>(pointer);
        auto iter = new_addr_map_.find(point_v);
        if (iter == new_addr_map_.end())
        {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        new_addr_map_.erase(iter);
        return true;
    }

    /*
    * @brief 打印内存信息
    */
    bool PrintMemoryInfo()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bool leaked = false;
        for (auto iter = new_addr_map_.begin(); iter != new_addr_map_.end(); iter++)
        {
            leaked = true;
            fprintf(stderr, "[内存泄漏分类统计] %s:%u size:%lu\n", iter->second.na_file, iter->second.na_line, iter->second.na_size);
        }
        if(!leaked)
        {
            fprintf(stderr, "[内存泄漏分类统计] 该程序没有内存泄漏\n");
        }
        return leaked;
    }

private:
    /*
    * @brief 定义存储申请内存的信息
    */
    struct NewAddr
    {
        const char* na_file;
        int32_t na_line;
        std::size_t na_size;
        NewAddr(const char* file, int32_t line, std::size_t size):na_file(file), na_line(line), na_size(size){}
    };
    // 定义互斥锁
    std::mutex mutex_;
    // 内存申请记录
    std::unordered_map<uint64_t, NewAddr> new_addr_map_;
};

// 定义全局互斥锁
// static std::mutex g_mutex;
/*
* @brief 定义指针链表
*/
struct NewPtrList
{
    NewPtrList* next;
    char filename[DEBUG_NEW_FILENAME_LEN];
    int32_t line;
    std::size_t size;
    NewPtrList()
    {
        next = nullptr;
        bzero(filename, DEBUG_NEW_FILENAME_LEN);
        line = 0;
        size = 0;
    }
};

// 定义内存记录
static RecordMemory g_record_memory;

bool CheckLeaks()
{
    return g_record_memory.PrintMemoryInfo();
}


void* operator new(std::size_t size, const char* file, int32_t line)
{
    std::size_t s = size + sizeof(NewPtrList);
    NewPtrList* ptr = (NewPtrList*)malloc(s);
    if(nullptr == ptr)
    {
        abort();
    }
    void* pointer = (char*)ptr + sizeof(NewPtrList);
    g_record_memory.AddNewAddr(ptr, size, file, line);
    return pointer;
}

void* operator new[](std::size_t size, const char* file, int32_t line)
{
    return operator new(size, file, line);
}

void* operator new(std::size_t size)
{
    return ::malloc(size);
}

void* operator new[](std::size_t size)
{
    return ::malloc(size);
}

void* operator new(std::size_t size, const std::nothrow_t&) throw()
{
    return operator new(size);
}
void* operator new[](std::size_t size, const std::nothrow_t&) throw()
{
    return operator new[](size);
}

void operator delete(void* pointer)
{
    if (nullptr == pointer)
    {
        return;
    }
    char* real_ptr = (char*)pointer - sizeof(NewPtrList);
    if(g_record_memory.RemoveNewAddr(real_ptr))
    {
        free(real_ptr);
    } else 
    {
        free(pointer);
    }

}

void operator delete[](void* pointer)
{
    operator delete(pointer);
}

void operator delete(void* pointer, const char* file, int32_t line)
{    
    operator delete(pointer);
}

void operator delete[](void* pointer, const char* file, int32_t line)
{
    operator delete(pointer);
}

void operator delete(void* pointer, const std::nothrow_t&)
{
    operator delete(pointer, "<Unknown>", 0);
}

void operator delete[](void* pointer, const std::nothrow_t&)
{
    operator delete(pointer, "<Unknown>", 0);
}

#define NEW new(__FILE__, __LINE__)

# else

#define CHECK_LEAKS
#define NEW new

#endif  // ENABLE_CHECK_LEAKS

