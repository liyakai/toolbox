#pragma once
#include <list>
#include <string>
#include "base_socket.h"
#include "epoll_define.h"
#include "socket_pool.h"
#include "udp_epoll_network.h"

class UdpSocket;
using UdpSocketPool = SocketPool<UdpSocket>;

/*
* 定义 UDP 地址
*/
class UdpAddress
{
public:
    /*
    * @brief 构造
    */
    UdpAddress()
        : id_(0)
        {}
    /*
    * @brief 构造
    * @param ip 地址
    * @param port 端口
    */
    UdpAddress(const std::string& ip, uint16_t port);
    /*
    * @brief 构造
    * @param address 地址
    */
    UdpAddress(const SocketAddress& address);
    /*
    * @brief 析构
    */
    ~UdpAddress() = default;
    /*
    * 重置
    */
    void Reset();
    /*
    * @brief 设置地址
    * @param address 地址
    */
    void SetAddress(const SocketAddress& address);
    /*
    * @brief 设置地址
    * @param ip 地址
    * @param port 端口
    */
    void SetAddress(const std::string& ip,  uint16_t port);
    /*
    * 取得地址
    */
    SocketAddress& GetAddress();
    /*
    * 取得地址的64位ID
    */
    uint64_t GetID() const;
private:
    uint64_t id_ = 0;           // 地址的64位ID
    SocketAddress address_;     // 地址
};

/*
* 定义一个UDP连接
*/
class UdpSocket : public BaseSocket
{
public:
    /*
    * 构造
    */
    UdpSocket();
    /*
    * 析构
    */
    ~UdpSocket();
    /*
    * 重置
    */
    void Reset() override;
    /*
    * 更新 epoll 事件
    * @params event_type 事件类型
    * @params ts 时间戳
    */
    void UpdateEpollEvent(SockEventType event_type, time_t ts) override;
    /*
    * @brief 监听(模拟)
    * @param ip 地址
    * @param port 端口
    */
    bool InitNewAccepter(const std::string& ip, uint16_t port);
    /*
    * @brief 连接
    * @param ip 地址
    * @param port 端口
    * @return 是否成功
    */
    bool InitNewConnecter(const std::string& ip, uint16_t port);
    /*
    * @brief 发送
    * @param buffer 数据指针
    * @param length 数据长度
    * @param address 目标地址
    */
    void SendTo(const char* buffer, std::size_t& length, SocketAddress& adress);
    /*
    * @brief 接收
    * @param buffer 数据指针
    * @param length 数据长度
    * @param address 来源地址
    * @return bool 是否成功
    */
    bool RecvFrom(char* buffer, std::size_t& length, SocketAddress& address);
    /*
    * @brief 获取管道类型
    */
    UdpType GetType();
    /*
    * @brief 设置 socket 池子
    */
    void SetSocketMgr(UdpSocketPool* sock_pool){p_sock_pool_ = sock_pool;}
    /*
    * @brief 设置 tcp_network
    */
    void SetEpollNetwork(UdpEpollNetwork* udp_network){ p_udp_network_ = udp_network; }
    /*
    * @brief 关闭套接字
    */
    void Close(ENetErrCode net_err, int32_t sys_err = 0);
    /*
    * @brief 设置远端地址
    */
    void SetRemoteAddress(const UdpAddress&& remote_address){ remote_address_ = remote_address; };
    /*
    * @brief 获取远端的地址
    */
    UdpAddress& GetRemoteAddress(){ return remote_address_; };
    /*
    * @brief 获取本地的地址
    */
    UdpAddress& GetLocalAddress(){ return local_address_; };
private:
    /*
    * @brief 绑定ip地址和端口
    * @param ip 地址
    * @param port 端口
    * @return 是否成功
    */
    bool Bind(const std::string& ip, uint16_t port);
    /*
    * @brief 处理错误事件
    */
    void UpdateError();
    /*
    * @brief 处理客户端数据
    */
    void UpdateRecv();
    /*
    * @brief 处理发送消息
    */
    void UpdateSend();
    /*
    * 处理接受客户端连接的情况
    */
    void UpdateAccept(const SocketAddress& address);
    /*
    *  初始化从accpet函数接收得来的socket
    */
    void InitAccpetSocket(UdpSocket* socket, const SocketAddress& address);
    /*
    * @brief 套接字接收数据
    */
    bool SocketRecv(int32_t socket_fd, char* data, size_t& size, const SocketAddress& address);
    /*
    * 套接字发送数据
    */
    bool SocketSend(int32_t socket_fd, const char* data, size_t& size, const SocketAddress& address);
private:
    // buff包
    struct Buffer
    {
        Buffer(const char* data, std::size_t size, SocketAddress& address);
        char buffer_[DEFAULT_CONN_BUFFER_SIZE]; // 缓冲区
        char* data_;            // 可读/写地址
        std::size_t size_;      // 可读/写长度
        SocketAddress address_; // 地址
    };
    using BufferList = std::list<Buffer*>;
    BufferList read_buffers_;   // 读缓冲区列表
    BufferList write_buffers_;  // 写缓冲区列表
    BufferList dead_buffers_;   // 需要销毁的缓冲区
    UdpAddress remote_address_; // 远端地址
    UdpAddress local_address_;  // 本地地址
    UdpType type_ = UdpType::UNKNOWN;              // 管道类型

    UdpEpollNetwork* p_udp_network_ = nullptr;      // 工作线程
    UdpSocketPool *p_sock_pool_ = nullptr;          // socket 池子

};