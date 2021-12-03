#pragma once

#include <stdint.h>
#include <time.h>
#include "src/network/net_imp/base_socket.h"
#include "src/tools/ringbuffer.h"
#include "socket_pool.h"
#include "src/network/net_imp/net_iocp/iocp_define.h"
#include "src/network/network_def.h"

class TcpEpollNetwork;
class TcpSocket;

using TCPSocketPool = SocketPool<TcpSocket>;

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
    * 设置socket状态
    */
    void SetSocketState(EIOSocketState state) { socket_state_ = state; }
    /*
    * 获取socket状态
    */
    EIOSocketState GetSocketState() { return socket_state_; }

    /*
    * 获取 persocket
    */
    PerSockContext& GetPerSocket() { return per_socket_; }
    /*
    * socket 是否有效
    */
    virtual bool IsSocketValid();
    /*
    * 设置 tcp_network
    */
    void SetNetwork(INetwork* tcp_network) { p_tcp_network_ = tcp_network; }
    /*
    * 获取远端地址信息
    */
    sockaddr_in* GetRemoteAddress(SOCKET&& listen_socket, char* accept_ex_buffer, int32_t buff_len);
    /*
    * 重置 PerSocket
    */
    inline void ResetPerSocket();
    /*
    * 重置接收 PerSocket
    */
    inline void ResetRecvPerSocket();
    /*
    * 重置发送 PerSocket
    */
    inline void ResetSendPerSocket();
    /*
    * 将 socket 建立与 iocp 的关联,只调用一次.
    */
    bool AssociateSocketToIocp();
    /*
    * 将 socket 重新监听读写事件
    */
    bool ReAddSocketToIocp(SockEventType event_type);
#elif defined(__linux__)
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
    /*
    * 发送错误,向主线程报告
    */
    void OnErrored(ENetErrCode err_code, int32_t err_no);

private:
    /*
    * 处理接受客户端连接的情况
    */
    void UpdateAccept();
    /*
    *  初始化从accpet函数接收得来的socket
    */
    bool InitAccpetSocket(TcpSocket* socket, int32_t socket_fd, std::string ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
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
    * 检查接收 ringbuffer 是否超限
    */
    bool CheckRecvRingBufferSize();
    /*
    * 检查发送 ringbuffer 是否超限
    */
    bool CheckSendRingBufferSize();
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
    PerSockContext per_socket_;                     //
    EIOSocketState socket_state_ = EIOSocketState::IOCP_CLOSE;
#elif defined(__linux__)
    SocketState socket_state_ = SocketState::SOCK_STATE_INVALIED;  // socket 状态
#endif

};