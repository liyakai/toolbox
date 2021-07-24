#include "memory_pool.h"

#define ENABLE_MEMORY_POOL 1

Chunk::Chunk()
{
}

Chunk::~Chunk()
{
    for (auto p : mem_list_)
    {
        delete p;
    }
    mem_list_.clear();
}

bool Chunk::SetChunkSize(std::size_t size)
{
    SetDebugPrint(false);
    if (0 == chunk_size_)
    {
        chunk_size_ = size;
        return true;
    }
    return false;
}

char* Chunk::GetMemory()
{
    bool is_empry = false;
    do 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (mem_list_.empty())
        {
            is_empry = true;
            break;
        }
        auto pointer = mem_list_.front();
        mem_list_.pop_front();
        // debug_free_set_.erase(pointer);     // debug code
        // debug_used_set_.insert(pointer);    // debug code
        // DebugPrint("Chunk::GetMemory");     // debug code
        return pointer;
    } while (true);
    if (is_empry)
    {   char* new_char = new char[chunk_size_];
        // debug_used_set_.insert(new_char);   // debug code
        return new_char;
    }
    return nullptr;
}

void Chunk::GiveBack(char* pointer, std::string debug_tag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // // debug code
    // auto iter_free = debug_free_set_.find(pointer);
    // if(iter_free != debug_free_set_.end())
    // {
    //     Print("[内存池.Chunk] 归还的内存块已经在free内存中,目前在重复归还!point:%p %s\n", pointer, debug_tag.c_str());
        
    //     return;
    // }
    // auto iter_used = debug_used_set_.find(pointer);
    // if(iter_used == debug_used_set_.end())
    // {
    //     Print("[内存池.Chunk] 归还的内存块不在used内存中!point:%p %s\n", pointer, debug_tag.c_str());
    //     return;
    // }
    // debug_free_set_.insert(pointer);
    // debug_used_set_.erase(pointer);
    // // end of debug code

    mem_list_.push_back(pointer);

    // DebugPrint("Chunk::GiveBack");  // debug code
}

std::size_t Chunk::Size()
{
    return mem_list_.size();
}

void Chunk::DebugPrint(std::string debug_tag)
{
    // Print("[内存池.Chunk] debug_tag: %s 内存池目前的容量为:%zu debug_free_set_ size:%zu debug_used_set_ size:%zu \n", debug_tag.c_str(), Size(), debug_free_set_.size(), debug_used_set_.size());
}

MemoryPool::MemoryPool()
{
}

MemoryPool::~MemoryPool()
{
    pool_.clear();
}

int32_t MemoryPool::RebuildNum(int32_t num)
{
	int32_t n = num - 1;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n < 0 ? 1 : n + 1;
}

char* MemoryPool::GetMemory(std::size_t size)
{
    size = RebuildNum(size + sizeof(std::uint32_t));
#if !ENABLE_MEMORY_POOL
    return new char[size];
#endif // !ENABLE_MEMORY_POOL
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

void MemoryPool::GiveBack(char* pointer, std::string debug_tag)
{
#if !ENABLE_MEMORY_POOL
    delete pointer;
    return;
#endif  // !ENABLE_MEMORY_POOL
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

void MemoryPool::DebugPrint()
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

