#pragma once
#include "net_imp_define.h"
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
    * @brief 设置 tcp_network
    * @param network 工作线程
    */
    void SetNetwork(INetwork* network){ p_network_ = network; }
    /*
    * 更新事件
    * @params event_type 事件类型
    * @params ts 时间戳
    */
    virtual void UpdateEvent(SockEventType event_type, time_t ts) = 0;
    /*
    * 获取事件类型
    * @return 可投递事件类型
    */
    int32_t GetEventType() const { return event_type_; };
    /*
    * 设置可投递类型
    */
    void SetSockEventType(int32_t type) { event_type_ = type; }
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    /*
    * 获取 socket id
    */
    SOCKET GetSocketID() { return socket_id_; }
    /*
    * 设置 socket id
    */
    void SetSocketID(SOCKET id) { socket_id_ = id; }
#elif defined(__linux__)
    /*
    * 获取 socket id
    */
    int32_t GetSocketID() { return socket_id_; }
    /*
    * 设置 socket id
    */
    void SetSocketID(int32_t id) { socket_id_ = id; }
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @return bool
    */
    bool IsCtrlAdd() { return is_ctrl_add_; }
    /*
    * 获取 socket 是否执行过 EPOLL_CTL_ADD
    * @params bool
    */
    void SetCtrlAdd(bool value) { is_ctrl_add_ = value; }
#endif

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
    virtual bool IsSocketValid();
protected:
    /*
    * 获取 socket 错误
    */
    int32_t GetSocketError();

protected:
    uint32_t conn_id_ = INVALID_CONN_ID;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    SOCKET socket_id_ = 0;
#elif defined(__linux__)
    int32_t socket_id_ = -1;                        // socket_id
    bool is_ctrl_add_ = false;                      // 是否已经执行过 EPOLL_CTL_ADD
#endif
    int32_t event_type_ = SOCKET_EVENT_INVALID;     // socket 可响应的事件类型
    INetwork* p_network_ = nullptr;                 // 工作线程
};