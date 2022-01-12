#pragma once
#include <new>
#include <mutex>
#include <unordered_map>
#include <map>
#include<string.h>

#define ENABLE_CHECK_LEAKS 1

#ifdef ENABLE_CHECK_LEAKS

// 定义文件名长度
#ifndef DEBUG_NEW_FILENAME_LEN
#define DEBUG_NEW_FILENAME_LEN 32
#endif // DEBUG_NEW_FILENAME_LEN

// 定义表的长度
#ifndef DEBUG_NEW_TABLESIZE
#define DEBUG_NEW_TABLESIZE (16*024*100)
#endif   // DEBUG_NEW_TABLESIZE

// 定义一个hash函数
#ifndef DEBUG_NEW_HASH
#define DEBUG_NEW_HASH(p) ((reinterpret_cast<uint64_t>(p) >> 8) % DEBUG_NEW_TABLESIZE)
#endif // DEBUG_NEW_HASH
// 统计打印输出内存使用情况
#define CHECK_LEAKS CheckLeaks();


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

// 定义全局内存
std::unordered_map<uint64_t, NewAddr> g_new_addr_map;

// 定义全局内存互斥锁
static std::mutex g_mem_mutex;
// 定义全局互斥锁
static std::mutex g_mutex;

void AddNewAddr(void* pointer, const char* file, int32_t line, std::size_t size)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_new_addr_map.insert(std::make_pair(reinterpret_cast<uint64_t>(pointer), NewAddr(file,line,size)));
}

void RemoveNewAddr(void* pointer)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_new_addr_map.erase(reinterpret_cast<uint64_t>(pointer));
}

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

static NewPtrList* g_new_ptr_list[DEBUG_NEW_TABLESIZE];
std::size_t list_size = 0;

bool CheckLeaks()
{
    struct ListEntry
    {
        char name[DEBUG_NEW_FILENAME_LEN + 8];
        int32_t size;
    };
    bool f_leaked = false;
    std::size_t index = 0;
    ListEntry* tmp_list = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        tmp_list = (ListEntry*)malloc(sizeof(ListEntry) * list_size);
        if (nullptr != tmp_list)
        {
            return false;
        }
        for (std::size_t i = 0; i < DEBUG_NEW_TABLESIZE; i++)
        {
            NewPtrList* ptr = g_new_ptr_list[i];
            if(nullptr == ptr)
            {
                continue;
            }
            f_leaked = true;
            while (ptr)
            {
                if(index < list_size)
                {
                    snprintf(tmp_list[index].name, DEBUG_NEW_FILENAME_LEN + 8,"%s:%d", ptr->filename, ptr->line);
                    tmp_list[index].size = ptr->size;
                    index++;
                }
                ptr = ptr -> next;
            }
        }
    }

    std::map<std::string, int32_t> leak_count;
    for (std::size_t i = 0; i < index; i++)
    {
        leak_count[tmp_list[i].name] += tmp_list[i].size;
    }
    free(tmp_list);

    // 打印
    for (auto iter = leak_count.begin(); iter != leak_count.end(); iter++)
    {
        fprintf(stderr, "[内存泄漏分类统计] %s size: %d\n", iter->first.c_str(), iter->second);
    }

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (auto iter = g_new_addr_map.begin(); iter != g_new_addr_map.end(); iter++)
        {
            f_leaked = true;
            fprintf(stderr, "[内存泄漏分类统计] %s:%u size:%lu", iter->second.na_file, iter->second.na_line, iter->second.na_size);
        }
    }
    if (!f_leaked)
    {
        fprintf(stderr, "[内存泄漏分类统计] 该程序没有任何内存泄漏");
    }
    
    return f_leaked;
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
    std::size_t hash_index = DEBUG_NEW_HASH(pointer);
    strncpy(ptr->filename, file, DEBUG_NEW_FILENAME_LEN - 1);
    ptr->line = line;
    ptr->size = size;

    {
        std::lock_guard<std::mutex> lock(g_mem_mutex);
        ptr->next = g_new_ptr_list[hash_index];
        g_new_ptr_list[hash_index] = ptr;
        list_size++;
    }
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
    std::size_t hash_index = DEBUG_NEW_HASH(pointer);
    NewPtrList* ptr_pre = nullptr;

    {
        std::lock_guard<std::mutex> lock(g_mem_mutex);
        NewPtrList* ptr = g_new_ptr_list[hash_index];
        while(ptr)
        {
            if((char*)ptr + sizeof(NewPtrList) == pointer)
            {
                if(nullptr == ptr_pre)
                {
                    g_new_ptr_list[hash_index] = ptr->next;
                } else
                {
                    ptr_pre->next = ptr->next;
                }

                list_size--;
                free(ptr);
                return;
            }
            ptr_pre = ptr;
            ptr = ptr->next;
        }
    }
    free(pointer);
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

#endif
