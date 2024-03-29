#pragma once

#ifdef LINUX_IO_URING

#include <bits/stdint-uintn.h>

namespace ToolBox
{

    // constexpr std::size_t MAX_MESSAGE_LEN = DEFAULT_RING_BUFF_SIZE;
    // constexpr std::size_t MAX_CONNECTIONS = MAX_SOCKET_COUNT;
    // constexpr std::size_t BUFFERS_COUNT = MAX_CONNECTIONS;

    constexpr std::size_t MAX_MESSAGE_LEN = 1024;
    constexpr std::size_t MAX_CONNECTIONS = 1024;
    constexpr std::size_t BUFFERS_COUNT = 1024;
    constexpr std::size_t URING_WAIT_MSECONDS = 2;

    /*
    * 定义 io_uring 的上下文状态.
    */
    enum class UringType : uint8_t
    {
        URING_ACCEPT,
        URING_READ,
        URING_WRITE,
        URING_PROV_BUF,
    };

    class BaseSocket;

    /*
    * 定义 io_uring 的上下文
    */
    struct UringIOContext
    {
        BaseSocket*     base_socket;        // 拥有此上下文的socket.
        SocketState     io_type;            // 当前的I/O类型. IOCP没有像epoll那样的EPOLLIN,EPOLLOUT,只能通过自身携带的类型做标记.
        uint16_t        bid;
        char*           buf;                // 异步缓冲区内存
        uint32_t        len;                // 异步缓冲区长度
    };

    /*
    * 定义 AcceptEx 函数相关
    */
    struct AcceptEx_t
    {
        uint32_t   socket_fd;               //  当前未决的客户端套接字  -AcceptEx
    };

    /*
    * 定义 io_uring 发送和接收的上下文
    */
    struct UringSockContext
    {
        AcceptEx_t*      accept_ex = nullptr;       // AcceptEx_t指针
        UringIOContext   io_recv;                   // 接收请求
        UringIOContext   io_send;                   // 发送请求
    };

}

#endif  // LINUX_IO_URING
