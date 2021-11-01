#include "socket_pool.h"

SocketPool::~SocketPool()
{ 
    UnInit();
};

bool SocketPool::Init(uint32_t max_count)
{
    if(0 == max_count)
    {
        return false;
    }
    if (max_count > UINT16_MAX - 1)
    {
        max_count = UINT16_MAX - 1;
    }
    max_socket_count_ = max_count;
    curr_index_ = 0;
    curr_cycle_ = 0;
    socket_vector_.resize(max_count, nullptr);
    for(uint16_t i = 0; i < max_count; i++)
    {
        active_slot_list_.emplace_back(i);
    }
    return true;
}

void SocketPool::UnInit()
{
    for(auto &es : free_list_)
    {
        delete es;
    }
    free_list_.clear();
    for(auto &es : socket_vector_)
    {
        if(nullptr != es)
        {
            delete es;
            es = nullptr;
        }
    }
    socket_vector_.clear();
}

SocketType* SocketPool::GetEpollSocket(uint32_t conn_id)
{
    uint16_t index = GetLoWord(conn_id);
    if(index < max_socket_count_)
    {
        return socket_vector_[index];
    }
    return nullptr;
}

SocketType* SocketPool::Alloc()
{
    uint32_t conn_id = NewID();
    if(INVALID_CONN_ID == conn_id)
    {
        return nullptr;
    }
    SocketType *socket = nullptr;
    if(!free_list_.empty())
    {
        socket = free_list_.front();
        free_list_.pop_front();
    } else 
    {
        socket = new SocketType;
    }
    socket->SetConnID(conn_id);
    uint16_t index = GetLoWord(conn_id);
    socket_vector_[index] = socket;
    return socket;
}

void SocketPool::Free(SocketType* socket)
{
    if(nullptr == socket)
    {
        return;
    }
    uint16_t index = GetLoWord(socket->GetConnID());
    socket_vector_[index] = nullptr;
    free_slot_list_.emplace_back(index);
    socket->Reset();
    free_list_.emplace_back(socket);

}

uint16_t SocketPool::NewIndex()
{
    if(active_slot_list_.empty())   // 剩余的下标用完了,交换
    {
        active_slot_list_.swap(free_slot_list_);
        curr_cycle_++;
    }

    if(active_slot_list_.empty())
    {
        return UINT16_MAX;
    }
    uint16_t index = active_slot_list_.front();
    active_slot_list_.pop_front();
    return index;
}

uint32_t SocketPool::NewID()
{
    uint16_t index = NewIndex();
    if(index == UINT16_MAX)
    {
        return INVALID_CONN_ID;
    }
    return MakeUint32(curr_cycle_, index);
}