#pragma once
#include <stdint.h>
#include <time.h>
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
    bool Init(SocketType type, uint32_t send_buff_len, uint32_t recv_buff_len);
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
    * @return 可投递事件类型
    */
    SockEventType GetEventType() const;
    /*
    * 获取文件描述符
    * @return 文件描述符
    */
    uint32_t GetSocketID() { return id_; }
    /*
    * 设置监听socket
    * @params listen_socket 监听的socket
    */
    void SetListenSocket(int32_t listen_socket){ listen_socket_ = listen_socket; }
    /*
    * 获取监听的socket
    * @return 监听的socket
    */
    int32_t GetListenSocket(){ return listen_socket_; }
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @return bool
    */
    bool IsCtrlAdd() { return is_ctrl_add_; }
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @params bool
    */
    void SetCtrlAdd(bool value);
    /*
    * 更新 epoll 事件
    * @params event_type 事件类型
    * @params ts 时间戳
    */
    void UpdateEpollEvent(SockEventType event_type, time_t ts);
private:
    /*
    * 处理接受客户端连接的情况
    */
   void UpdateAccept();
    /*
    * 接受客户端连接
    * @return 客户端连接文件描述符
    */
    int Accept();
    /*
    *  初始化新的socket
    */
   

private:
    uint32_t id_ = 0; // socket_id
    uint32_t ip_ = 0;
    uint16_t port_ = 0;
    int32_t listen_socket_ = 0;

    SocketState socket_state_ = SocketState::SOCK_STATE_INVALIED;  // socket 状态
    SockEventType event_type_; // 可投递事件类型
    time_t last_recv_ts_ = 0;   // 最后一次读到数据的时间戳
    bool is_ctrl_add_ = false; // 是否已经执行过 EPOLL_CTL_ADD
};