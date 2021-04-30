#pragma once

#include "singleton.h"
#include <list>
#include <unordered_map>

/*
*  ��ͬ��С�ڴ��Ĺ���
*/
class Chunk
{
public:
    /*
    * ���캯��
    */
    Chunk();
    /*
    * ����
    */
    ~Chunk();
    /*
    * �����ڴ���С[ֻ�е�һ�����÷���ֵ��Ч]
    * @param size �ڴ���С
    */
    bool SetChunkSize(std::size_t size);
    /*
    * ��ȡ�ڴ��
    */
    char* GetMemory();
    /*
    * �黹�ڴ��
    */
    void GiveBack(char* pointer);
    /*
    * ʣ���ڴ�����
    */
    std::size_t Size();

private:
    std::size_t chunk_size_ = 0;
    std::mutex mutex_;
    std::list<char*> mem_list_;
};

/*
* �ڴ��
*/
class MemoryPool
{
public:
    /*
    * ���캯��
    */
    MemoryPool();
    /*
    * ����
    */
    ~MemoryPool();
    /*
	* �����ڴ�
	* @param �����ڴ�Ĵ�С
	*/
    char* GetMemory(std::size_t size);
    /*
    * �黹�ڴ�
    * @param �黹�ڴ��ָ��
    */
    void GiveBack(char* pointer);
	/*
	* ���Դ�ӡ
	*/
	void DebugPrint();
private:
    /*
    * ����ڵ���(С�ڵ���)һ��������С2�����㷨
    * [�㷨ԭ���] https://blog.csdn.net/Kakarotto_/article/details/108958843
    */
    int32_t RebuildNum(int32_t num);
private:
    std::unordered_map<std::size_t, Chunk> pool_;
    bool is_debug_print_ = false;
};

#define MemPoolMgr Singleton<MemoryPool>::instance()
