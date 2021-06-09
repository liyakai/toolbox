#include "epoll_socket_mgr.h"

EpollSocketPool::EpollSocketPool(){}
EpollSocketPool::~EpollSocketPool()
{
    UnInit();
}
bool EpollSocketPool::Init(uint32_t max_count)
{
    if(0 == max_count)
    {
        return false;
    }
    if (max_count > UINT16_MAX)
    {
        max_count = UINT16_MAX;
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

void EpollSocketPool::UnInit()
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

EpollSocket* EpollSocketPool::GetEpollSocket(uint32_t conn_id)
{
    return nullptr;
}