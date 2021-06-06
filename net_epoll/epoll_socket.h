#pragma once
#include <stdint.h>
#include "epoll_define.h"
/*
* 定义每一个连接
*/
class EpollSocket
{
public:
    /*
    * 构造
    */
    EpollSocket();
    /*
    * 析构
    */
    ~EpollSocket();
    /*
    * 初始化
    */
    bool Init(SocketType type, uint32_t send_buff_len, uint32_t  recv_buff_len);
    /*
    * 逆初始化
    */
    void UnInit();
    /*
    * 重置
    */
    void Reset();
    /*
    * 获取事件类型
    * @params 可投递事件类型
    */
    SockEventType GetEventType() const;

private:
    uint32_t id_ = 0;   // socket_id
    uint32_t ip_ = 0;
    uint16_t port_ = 0;

    EpollData epoll_data_;

    SockEventType event_type_;  // 可投递事件类型
};