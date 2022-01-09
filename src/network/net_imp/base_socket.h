#pragma once
#include "net_imp_define.h"
#include "network.h"

namespace ToolBox{

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
    * @brief 监听(模拟)
    * @param ip 地址
    * @param port 端口
    */
    virtual bool InitNewAccepter(const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
    /*
    * @brief 连接
    * @param ip 地址
    * @param port 端口
    * @return 是否成功
    */
    virtual bool InitNewConnecter(const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
    /*
    * @brief 发送 [原生UDP发送接口,不适用于kcp等协议的发送入口]
    * @param buffer 数据指针
    * @param length 数据长度
    * @param address 目标地址
    */
    virtual void Send(const char* buffer, std::size_t length) = 0;
    /*
    * 获取socket状态
    */
    virtual SocketState GetSocketState() { return SOCK_STATE_INVALIED; }
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
    * 发送错误,向主线程报告
    */
    void OnErrored(ENetErrCode err_code, int32_t err_no);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    /*
    * 获取 socket id
    */
    SOCKET GetSocketID() { return socket_id_; }
    /*
    * 设置 socket id
    */
    void SetSocketID(SOCKET id) { socket_id_ = id; }
    /*
    * 获取 persocket
    */
    virtual PerSockContext* GetPerSocket() { return nullptr; }
    /*
    * 重置接收 PerSocket
    */
    virtual void ResetRecvPerSocket() {};
    /*
    * 重置发送 PerSocket
    */
    virtual void ResetSendPerSocket(){};
    /*
    * 将 socket 建立与 iocp 的关联,只调用一次.
    */
    virtual bool AssociateSocketToIocp() { return true; };
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
#elif defined(__APPLE__)
    /*
    * 获取 socket id
    */
    int32_t GetSocketID() { return socket_id_; }
    /*
    * 设置 socket id
    */
    void SetSocketID(int32_t id) { socket_id_ = id; }

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
    virtual void Close(ENetErrCode net_err, int32_t sys_err);
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
#elif defined(__APPLE__)
    int32_t socket_id_ = -1;                        // socket_id
#endif
    int32_t event_type_ = SOCKET_EVENT_INVALID;     // socket 可响应的事件类型
    INetwork* p_network_ = nullptr;                 // 工作线程
};

};  // ToolBox