#pragma once
#include <stdint.h>
#include <deque>
#include <vector>
#include <set>
#include "epoll_define.h"

/*
* socket 池子
*/
template<typename SocketType>
class SocketPool
{
public:
    /*
    * 构造
    */
    SocketPool(){};
    /*
    * 析构
    */
    ~SocketPool()
    { 
        UnInit();
    };
    /*
    * 初始化
    * @param max_count 最多可以连接的用户数
    */
    bool Init(uint32_t max_count)
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
    /*
    * 逆初始化
    */
    void UnInit()
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
    /*
    * 获取一个 socket
    * @param conn_id 连接ID
    * @return socket
    */
    SocketType* GetEpollSocket(uint32_t conn_id)
    {
        uint16_t index = GetLoWord(conn_id);
        if(index < max_socket_count_)
        {
            return socket_vector_[index];
        }
        return nullptr;
    }
    /*
    * 分配新的 Socket
    */
    SocketType* Alloc()
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
    /*
    * 归还用过的 socket
    */
    void Free(SocketType* socket)
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
private:
    /*
    * 生成新的 index
    */
    uint16_t NewIndex()
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
    /*
    * 生成新的 ID
    */
    uint32_t NewID()
    {
        uint16_t index = NewIndex();
        if(index == UINT16_MAX)
        {
            return INVALID_CONN_ID;
        }
        return MakeUint32(curr_cycle_, index);
    }
    /*
    * 工具函数 获取低16位
    * @param val 32位无符号数字
    * @return 低16位无符号数字
    */
    uint16_t GetLoWord(uint32_t val){ return (uint16_t)(val & 0xFFFF); }
    /*
    * 工具函数,拼接两个uint16_t为一个uint32_t
    */
    uint32_t MakeUint32(uint16_t hi_word, uint16_t lo_word){return ((uint32_t)hi_word | lo_word);}
private:
    uint32_t max_socket_count_ = 0; // 池子最大数量
    uint16_t curr_index_ = 0;       // 当前用到的索引值
    uint16_t curr_cycle_ = 0;       // 当前用到的第几轮

    using EpollSocketVector = std::vector<SocketType*>;
    using EpollSocketList = std::deque<SocketType*>;
    using SlotIndexList = std::deque<uint16_t>;

    EpollSocketVector socket_vector_;       // socket 管理容器
    EpollSocketList free_list_;             // 回收的实体socket对象
    SlotIndexList active_slot_list_;        // 可用的 socket 索引
    SlotIndexList free_slot_list_;          // 回收的管理器索引
};