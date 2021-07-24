#pragma once

#include "singleton.h"
#include <list>
#include <unordered_map>
#include <set>
#include "debug_print.h"
/*
*  相同大小内存块的管理
*/
class Chunk : public DebugPrint
{
public:
    /*
    * 构造函数
    */
    Chunk();
    /*
    * 析构
    */
    ~Chunk();
    /*
    * 设置内存块大小[只有第一次设置非零值有效]
    * @param size 内存块大小
    */
    bool SetChunkSize(std::size_t size);
    /*
    * 获取内存块
    */
    char* GetMemory();
    /*
    * 归还内存块
    */
    void GiveBack(char* pointer, std::string debug_tag = "");
    /*
    * 剩余内存块个数
    */
    std::size_t Size();
    /*
    * Debug status
    */
    void DebugPrint(std::string debug_tag = "");

private:
    std::size_t chunk_size_ = 0;
    std::mutex mutex_;
    std::list<char*> mem_list_;
    // std::set<char*> debug_free_set_;        // debug code
    // std::set<char*> debug_used_set_;        // debug code
};

/*
* 内存池
*/
class MemoryPool : public DebugPrint
{
public:
    /*
    * 构造函数
    */
    MemoryPool();
    /*
    * 析构
    */
    ~MemoryPool();
    /*
	* 申请内存
	* @param 申请内存的大小
	*/
    char* GetMemory(std::size_t size);
    /*
    * 归还内存
    * @param 归还内存的指针
    */
    void GiveBack(char* pointer, std::string debug_tag = "");
	/*
	* 调试打印
	*/
	void DebugPrint();
private:
    /*
    * 求大于等于(小于等于)一个整数最小2次幂算法
    * [算法原理见] https://blog.csdn.net/Kakarotto_/article/details/108958843
    */
    int32_t RebuildNum(int32_t num);
private:
    std::unordered_map<std::size_t, Chunk> pool_;
};

#define MemPoolMgr Singleton<MemoryPool>::Instance()
