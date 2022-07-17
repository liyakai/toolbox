#pragma once

#ifdef __linux__

#include <list>
#include <string>
#include "network/net_imp/base_socket.h"
#include "net_imp_define.h"
#include "network/net_imp/socket_pool.h"
//#include "udp_epoll_network.h"
#include "kcp/ikcp.h"

namespace ToolBox
{

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
        {
        }
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
        void UpdateEvent(SockEventType event_type, time_t ts) override;
        /*
        * @brief 监听(模拟)
        * @param ip 地址
        * @param port 端口
        */
        bool InitNewAccepter(const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
        /*
        * @brief 连接
        * @param ip 地址
        * @param port 端口
        * @return 是否成功
        */
        bool InitNewConnecter(const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
        /*
        * @brief 发送 [原生UDP发送接口,不适用于kcp等协议的发送入口]
        * @param buffer 数据指针
        * @param length 数据长度
        * @param address 目标地址
        */
        void Send(const char* buffer, std::size_t length);
        /*
        * @brief 发送 [KCP发送入口]
        * @param buffer 数据指针
        * @param length 数据长度
        * @param address 目标地址
        */
        void KcpSendTo(const char* buffer, std::size_t length);
        /*
        * @brief 获取管道类型
        */
        UdpType GetType();
        /*
        * @brief 设置 socket 池子
        */
        void SetSocketMgr(UdpSocketPool* sock_pool)
        {
            p_sock_pool_ = sock_pool;
        }
        /*
        * @brief 关闭套接字
        */
        void Close(ENetErrCode net_err, int32_t sys_err = 0);
        /*
        * @brief 设置远端地址
        */
        void SetRemoteAddress(const UdpAddress&& remote_address)
        {
            remote_address_ = remote_address;
        };
        /*
        * @brief 获取远端的地址
        */
        UdpAddress& GetRemoteAddress()
        {
            return remote_address_;
        };
        /*
        * @brief 获取远端地址和端口拼成的ID
        */
        uint64_t GetRemoteAddressID()
        {
            return remote_address_.GetID();
        }
        /*
        * @brief 获取本地的地址
        */
        UdpAddress& GetLocalAddress()
        {
            return local_address_;
        };
        /*
        * @brief 获取本地地址和端口拼成的ID
        */
        uint64_t GetLocalAddressID()
        {
            return local_address_.GetID();
        }
    public:
        /*
        * @brief 开启Kcp模式
        */
        void OpenKcpMode();
        /*
        * @brief 获取kcp句柄
        */
        ikcpcb* GetKcp()
        {
            return kcp_;
        }
        /*
        * @brief 设置udp类型
        */
        void SetType(UdpType type)
        {
            type_ = type;
        };
        /*
        * @brief KCP update
        */
        void KcpUpdate(std::time_t current);
        /*
        * @brief KCP 接收数据
        * @param buffer 数据指针
        * @param length 数据长度
        * @param address 数据来源地址
        */
        void KcpRecv(const char* buffer, std::size_t length, const UdpAddress&& address);
    private:
        /*
        * @brief 绑定ip地址和端口.[用于监听]
        * @param ip 地址
        * @param port 端口
        * @return 是否成功
        */
        bool Bind(const std::string& ip, uint16_t port);
        /*
        * @brief 绑定到本地任意地址和端口.[用于主动连接]
        * @return 是否成功
        */
        bool Bind();
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
        * @brief 处理接受客户端连接的情况
        * @param address 远端地址
        * @return UdpSocket* 新socket,如果失败则为nullptr
        */
        UdpSocket* UpdateAccept(const SocketAddress& address);
        /*
        *  初始化从accpet函数接收得来的socket
        */
        void InitAccpetSocket(UdpSocket* socket, const SocketAddress& address);
        /*
        * @brief 套接字接收数据
        */
        bool SocketRecv(int32_t socket_fd, char* data, size_t& size, SocketAddress& address);
        /*
        * 套接字发送数据
        */
        bool SocketSend(int32_t socket_fd, const char* data, size_t& size);
        /*
        * @brief kcp 用来执行发送的回调函数
        */
        static int32_t Output(const char* buf, int32_t len, ikcpcb* kcp, void* user);
    private:
        // buff包
        struct Buffer
        {
            Buffer(const char* data, std::size_t size);
            char buffer_[DEFAULT_CONN_BUFFER_SIZE]; // 缓冲区
            char* data_;            // 可读/写地址
            std::size_t size_;      // 可读/写长度
        };
        using BufferList = std::list<Buffer*>;
        // BufferList recv_list_;      // 接收缓冲区列表
        BufferList send_list_;      // 接收缓冲区列表
        UdpAddress remote_address_; // 远端地址
        UdpAddress local_address_;  // 本地地址
        UdpType type_ = UdpType::UNKNOWN;               // 管道类型
        ikcpcb* kcp_ = nullptr;     // kcp实例
        UdpSocketPool* p_sock_pool_ = nullptr;          // socket 池子
    };

};  // ToolBox

#endif // __linux__