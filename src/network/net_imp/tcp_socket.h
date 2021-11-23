#pragma once

#include <stdint.h>
#include <time.h>
#include "src/network/net_imp/base_socket.h"
#include "src/tools/ringbuffer.h"
#include "socket_pool.h"

class TcpEpollNetwork;
class TcpSocket;

using TCPSocketPool = SocketPool<TcpSocket>;



/*
* 定义 per-I/O 数据
*/
struct PerIO_t
{
    OVERLAPPED over_lapped; // windows 重叠I/O数据结构
    EIOSocketState    io_type;     // 当前的I/O类型
};

/*
* 定义 AcceptEx 函数相关
*/
struct AcceptEx_t
{
    ACCEPTEX accept_ex_fn;      // AcceptEx 函数指针
    SOCKET   socket_fd;         // 当前未决的客户端套接字  -AcceptEx
    char     buffer[DEFAULT_CONN_BUFFER_SIZE];           // 参数 AcceptEx
};

/*
* 定义 per-socket 数据
*/
template<typename SocketType>
struct PerSock_t
{
    SocketType* net_socket;     // socket 指针
    AcceptEx_t* accept_ex_info; // AcceptEx_t指针
    PerIO_t   io_recv;
    PerIO_t   io_send;
};

/*
* 定义一个TCP连接
*/
class TcpSocket : public BaseSocket
{
public:
    /*
    * 构造
    */
    TcpSocket();
    /*
    * 析构
    */
    ~TcpSocket();
    /*
    * 初始化
    */
    bool Init(int32_t send_buff_len, int32_t recv_buff_len);
    /*
    * 逆初始化
    */
    void UnInit();
    /*
    * 重置
    */
    void Reset();
    // /*
    // * 设置远端IP
    // */
    // void SetIP(std::string ip){ip_ = ip;} 
    // /*
    // * 设置远端端口
    // */
    // void SetAddressPort(uint16_t port){ port_ = port; }


    /*
    * 设置 socket 池子
    */
    void SetSocketMgr(TCPSocketPool* sock_pool){p_sock_pool_ = sock_pool;}
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    /*
    * 获取 AcceptEx 指针
    */
    ACCEPTEX& GetAcceptEx() { return accept_ex_fn_; }
    /*
    * 设置socket状态
    */
    void SetSocketState(EIOSocketState state) { socket_state_ = state; }
    /*
    * 获取socket状态
    */
    EIOSocketState GetSocketState() { return socket_state_; }
    /*
    * 获取缓冲区
    */
    char* GetBuffer() { return buffer; }
    /*
    * 获取重叠结构
    */
    OVERLAPPED& GetOverLapped() { return over_lapped_; }
    /*
    * 获取重叠结构指针
    */
    OVERLAPPED* GetOverLappedPtr() { return &over_lapped_; }
    /*
    * 设置 当前未决的客户端套接字SocketID 
    */
    void SetAcceptExSocketId(SOCKET socket) { accept_ex_socket_id_ = socket; }
#elif defined(__linux__)
    /*
    * 设置 tcp_network
    */
    void SetEpollNetwork(TcpEpollNetwork* tcp_network){ p_tcp_network_ = tcp_network; }
    /*
    * 设置socket状态
    */
    void SetSocketState(SocketState state) { socket_state_ = state; }
#endif
    /*
    * 更新 epoll 事件
    * @params event_type 事件类型
    * @params ts 时间戳
    */
    void UpdateEvent(SockEventType event_type, time_t ts) override;
    /*
    * 初始化新的监听器
    * @param ip 监听IP
    * @param port 监听端口
    * @retval 初始化是否成功
    */
    bool InitNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
    /*
    * 初始化新的连接器
    * @param ip 连接IP
    * @param port 连接端口
    * @retval 初始化是否成功
    */
    bool InitNewConnecter(const std::string &ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
    /*
    * 发送数据
    * @param data 发送数据指针
    * @param len 发送数据长度
    */
    void Send(const char* data, size_t len);
    /*
    * 关闭套接字
    */
    void Close(ENetErrCode net_err, int32_t sys_err = 0);

private:
    /*
    * 处理接受客户端连接的情况
    */
    void UpdateAccept();
    /*
    *  初始化从accpet函数接收得来的socket
    */
    void InitAccpetSocket(TcpSocket* socket, int32_t socket_fd, std::string ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
    /*
    * 处理客户端数据的情况
    */
    void UpdateRecv();
    /*
    * 套接字接收数据
    */
    int32_t SocketRecv(int32_t socket_fd, char* data, size_t size);
    /*
    * 处理接收到的数据
    */
    ErrCode ProcessRecvData();
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
    int32_t SocketSend(int32_t socket_fd, const char* data, size_t size);
    /*
    * 处理错误事件
    */
    void UpdateError();
    /*
    * 设置 非阻塞
    */
    int32_t SetNonBlocking(int32_t fd);
    /*
    * 关闭 keep alive
    */
    int32_t SetKeepaliveOff(int32_t fd);
    /*
    * 关闭延迟发送
    */
    int32_t SetNagleOff(int32_t fd);
    /*
    * 关闭 TIME_WAIT
    */
    int32_t SetLingerOff(int32_t fd);
    /*
    * 让端口释放后立即就可以被再此使用
    */
    int32_t SetReuseAddrOn(int32_t fd);
    /*
    * 设置 TCP_DEFER_ACCEPT 
    */
    int32_t SetDeferAccept(int32_t fd);
    /*
    * 设置 TCP buffer 的大小
    */
    int32_t SetTcpBuffSize(int32_t fd); 
    /*
    * @brief 获取系统错误消息
    */
    int32_t GetSysErrNo();

private:
    // std::string ip_;
    // uint16_t port_ = 0;
    INetwork* p_tcp_network_ = nullptr;      // 工作线程

    TCPSocketPool *p_sock_pool_ = nullptr;          // socket 池子

    int32_t send_buff_len_ = 0;                     // 接收缓冲区大小
    int32_t recv_buff_len_ = 0;                     // 接收缓冲区大小
    RingBuffer<char, DEFAULT_RING_BUFF_SIZE> send_ring_buffer_;
    RingBuffer<char, DEFAULT_RING_BUFF_SIZE> recv_ring_buffer_;
    time_t last_recv_ts_ = 0;                       // 最后一次读到数据的时间戳

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    ACCEPTEX accept_ex_fn_ = nullptr;      // AcceptEx 函数指针
    SOCKET accept_ex_socket_id_ = 0;                 // 当前未决的客户端套接字
    EIOSocketState socket_state_ = EIOSocketState::IOCP_CLOSE;
    char buffer[DEFAULT_CONN_BUFFER_SIZE];           // 参数 AcceptEx
    OVERLAPPED over_lapped_; // windows 重叠I/O数据结构
#elif defined(__linux__)
    SocketState socket_state_ = SocketState::SOCK_STATE_INVALIED;  // socket 状态
#endif

};
