#pragma once
#include <stdint.h>
#include <deque>
#include <vector>
#include "epoll_socket.h"

/*
* socket 池子
*/
class EpollSocketPool
{
public:
    /*
    * 构造
    */
    EpollSocketPool();
    /*
    * 析构
    */
    ~EpollSocketPool();
    /*
    * 初始化
    * @param max_count 最多可以连接的用户数
    */
    bool Init(uint32_t max_count);
    /*
    * 逆初始化
    */
    void UnInit();
    /*
    * 获取一个 socket
    * @param conn_id 连接ID
    * @return socket
    */
   EpollSocket* GetEpollSocket(uint32_t conn_id);
private:
    uint32_t max_socket_count_ = 0; // 池子最大数量
    uint16_t curr_index_ = 0;       // 当前用到的索引值
    uint16_t curr_cycle_ = 0;       // 当前用到的第几轮

    using EpollSocketVector = std::vector<EpollSocket*>;
    using EpollSocketList = std::deque<EpollSocket*>;
    using SlotIndexList = std::deque<uint16_t>;

    EpollSocketVector socket_vector_;   // socket 管理容器
    EpollSocketList free_list_;         // 回收的实体socket对象
    SlotIndexList active_slot_list_;    // 可用的 socket 管理器索引
    SlotIndexList free_slot_list_;      // 回收的管理器索引

};