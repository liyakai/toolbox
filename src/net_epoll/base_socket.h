#pragma once
#include "epoll_define.h"
#include "network.h"

/*
* 定义 socket 基类
*/
class BaseSocket
{
public:
    /*
    * 构造
    */
    BaseSocket();
    /*
    * 析构
    */
    virtual ~BaseSocket();
    /*
    * 重置
    */
    virtual void Reset();
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @return bool
    */
    bool IsCtrlAdd() { return is_ctrl_add_; }
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @params bool
    */
    void SetCtrlAdd(bool value){ is_ctrl_add_ = value;}
    /*
    * 获取事件类型
    * @return 可投递事件类型
    */
    int32_t GetEventType() const { return event_type_; };
    /*
    * 设置可投递类型
    */
    void SetSockEventType(int32_t type) { event_type_ = type; }
    /*
    * 获取 socket id
    */
    int32_t GetSocketID() { return socket_id_; }
    /*
    * 设置 socket id
    */
    void SetSocketID(int32_t id) { socket_id_ = id; }
    /* 
    * 获取连接ID
    */
    uint32_t GetConnID(){ return conn_id_; }
    /*
    * 设置 分配的连接ID
    */
    void SetConnID(uint32_t id){ conn_id_ = id; }
    /*
    * @brief 关闭
    */
    void Close();
    /*
    * socket 是否有效
    */
    bool IsSocketValid();
protected:
    uint32_t conn_id_ = INVALID_CONN_ID;
    int32_t socket_id_ = -1; // socket_id
    bool is_ctrl_add_ = false;                      // 是否已经执行过 EPOLL_CTL_ADD
    int32_t event_type_ = SOCKET_EVENT_INVALID;     // 可投递事件类型
};