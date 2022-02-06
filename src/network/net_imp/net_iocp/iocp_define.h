#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

namespace ToolBox{

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx 函数指针,详见MSDN
// MSDN: The number of bytes reserved for the local address information. 
// This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
constexpr std::size_t ACCEPTEX_ADDR_SIZE = sizeof(SocketAddress) + 16;
constexpr std::size_t ACCEPTEX_BUFF_SIZE = 2014;

/*
* 定义 per-I/O 数据
*/
struct PerIOContext
{
    OVERLAPPED      over_lapped;    // windows 重叠I/O数据结构
    WSABUF          wsa_buf;        // 存储数据的缓冲区,用来给重叠操作传递参数.
    SocketState     io_type;        // 当前的I/O类型. IOCP没有像epoll那样的EPOLLIN,EPOLLOUT,只能通过自身携带的类型做标记.
};

/*
* 定义 AcceptEx 函数相关
*/
struct AcceptEx_t
{
    ACCEPTEX accept_ex_fn = nullptr;      // AcceptEx 函数指针
    SOCKET   socket_fd;         //  当前未决的客户端套接字  -AcceptEx
    char     buffer[ACCEPTEX_BUFF_SIZE];           // 参数 AcceptEx
};

/*
* 定义 per-socket 数据
*/
struct PerSockContext
{
    AcceptEx_t* accept_ex = nullptr;      // AcceptEx_t指针
    PerIOContext   io_recv;     // 接收请求
    PerIOContext   io_send;     // 发送请求
};

};  // ToolBox

#endif
