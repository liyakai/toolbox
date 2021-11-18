#pragma once

#include "src/network/net_imp/net_imp_define.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

enum EIOType
{
    IOT_ACCEPT  = 1,    // 新连接
    IOT_CONNECT = 2,    // 连接建立
    IOT_RECV    = 4,    // 接收
    IOT_SEND    = 8,    // 发送
    IOT_CLOSE   = 16,   // 关闭
};

/*
* 定义 per-I/O 数据
*/
struct PerIO_t
{
    OVERLAPPED over_lapped; // windows 重叠I/O数据结构
    EIOType    io_type;     // 当前的I/O类型
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
* 定义管理 iocp 的类
*/
class IocpCtrl
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    IocpCtrl();
    /*
    * 创建 iocp
    * @return 是否成功
    */
    bool CreateIocp();
    /*
    * 销毁 iocp
    */
    void Destroy();
    /*
    * 执行一次 iocp
    */
    template<typename SocketType>
    bool RunOnce()
    {
        time_t      time_stamp = time(0);    // 时间戳
        DWORD       bytes = 0;
        PerSock_t<SocketType>* per_sock = nullptr;
        PerIO_t* per_io = nullptr;
        bool error = GetQueuedCompletionStatus(iocp_fd_, &bytes, (PULONG_PTR)&per_sock, (LPOVERLAPPED*)&per_io, 2);
        auto last_error = GetLastError();
        if (false == error)
        {
            if (WAIT_TIMEOUT == last_error || ERROR_OPERATION_ABORTED == last_error)
            {
                return true;
            }
        }
        if (nullptr == per_sock || nullptr == per_io || nullptr == per_sock->net_socket)
        {
            return false;
        }
        if ((per_io->io_type & EIOType::IOT_RECV) || (per_io->io_type & EIOType::IOT_ACCEPT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        } else if ((per_io->io_type & EIOType::IOT_SEND) || (per_io->io_type & EIOType::IOT_CONNECT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        }

        return true;
    }
private:
    HANDLE iocp_fd_;            // iocp 文件描述符
};

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)