#pragma once
#include <stdint.h>
#include <time.h>
#include "epoll_define.h"
#include "src/tools/ringbuffer.h"

class TcpNetwork;
class EpollSocketPool;
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
    SockEventType GetEventType() const { return event_type_; };
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
    * 设置远端IP
    */
    void SetIP(std::string ip){ip_ = ip;} 
    /*
    * 设置远端端口
    */
    void SetPort(uint16_t port){ port_ = port; }
    /*
    * 设置socket状态
    */
    void SetState(SocketState state) { socket_state_ = state; }
    /*
    * 设置可投递类型
    */
    void SetSockEventType(SockEventType type) { event_type_ = type; }
    /*
    * 设置 socket 池子
    */
    void SetSocketMgr(EpollSocketPool* sock_pool){p_sock_pool_ = sock_pool;}
    /*
    * 设置 tcp_network
    */
    void SetTcpNetwork(TcpNetwork* tcp_network){ p_tcp_network_ = tcp_network; }
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
    /*
    * 初始化新的监听器
    * @param ip 监听IP
    * @param port 监听端口
    * @retval 初始化是否成功
    */
    bool InitNewAccepter(const std::string& ip, const uint16_t port);
    /*
    * 初始化新的连接器
    * @param ip 连接IP
    * @param port 连接端口
    * @retval 初始化是否成功
    */
    bool InitNewConnecter(const std::string &ip, uint16_t port);
private:
    /*
    * 处理接受客户端连接的情况
    */
    void UpdateAccept();
    /*
    *  初始化从accpet函数接收得来的socket
    */
    void InitAccpetSocket(EpollSocket* socket, int socket_fd, std::string ip, uint16_t port);
    /*
    * 处理客户端数据的情况
    */
    void UpdateRecv();
    /*
    * 关闭套接字
    */
    void Close();
    /*
    * 套接字接收数据
    */
    size_t SocketRecv(int socket_fd, char* data, size_t size);
    /*
    * 处理接收到的数据
    */
    void ProcessRecvData();
    /*
    * 处理主动链接
    */
    void UpdateConnect();
    /*
    * 处理发送消息
    */
    void UpdateSend();
    /*
    * 套接字发送数据
    */
    size_t SocketSend(int socket_fd, const char* data, size_t size);
    /*
    * 设置 非阻塞
    */
    int SetNonBlocking(int fd);
    /*
    * 关闭 keep alive
    */
    int SetKeepaliveOff(int fd);
    /*
    * 关闭延迟发送
    */
    int SetNagleOff(int fd);
    /*
    * 关闭 TIME_WAIT
    */
    int SetLingerOff(int fd);
    /*
    * 让端口释放后立即就可以被再此使用
    */
    int SetReuseAddrOn(int fd);
    /*
    * 设置 TCP_DEFER_ACCEPT 
    */
    int SetDeferAccept(int fd);

private:
    uint32_t conn_id_ = INVALID_CONN_ID;
    int32_t socket_id_ = -1; // socket_id
    std::string ip_;
    uint16_t port_ = 0;

    TcpNetwork* p_tcp_network_ = nullptr;     // 工作线程
    EpollSocketPool *p_sock_pool_ = nullptr;  // socket 池子

    SocketState socket_state_ = SocketState::SOCK_STATE_INVALIED;  // socket 状态
    SockEventType event_type_; // 可投递事件类型
    int recv_buff_len_ = 0;     // 接收buff大小
    RingBuffer<char, DEFAULT_RING_BUFF_SIZE> recv_ring_buffer_;
    RingBuffer<char, DEFAULT_RING_BUFF_SIZE> send_ring_buffer_;
    time_t last_recv_ts_ = 0;   // 最后一次读到数据的时间戳
    bool is_ctrl_add_ = false; // 是否已经执行过 EPOLL_CTL_ADD
};