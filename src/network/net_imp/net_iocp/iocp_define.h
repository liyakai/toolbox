#pragma once

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx 函数指针,详见MSDN

// MSDN: The number of bytes reserved for the local address information. 
// This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
constexpr std::size_t ACCEPTEX_ADDR_SIZE = sizeof(sockaddr_in) + 16;
constexpr std::size_t ACCEPTEX_BUFF_SIZE = 2014;



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

/*
* IOCP下socket的状态
*/
enum EIOSocketState
{
    IOCP_ACCEPT = 1,    // 监听
    IOCP_CONNECT = 2,    // 连接建立
    IOCP_RECV = 4,    // 接收
    IOCP_SEND = 8,    // 发送
    IOCP_CLOSE = 16,   // 关闭
};


/*
* 定义 per-I/O 数据
*/
struct PerIOContext
{
    OVERLAPPED      over_lapped;    // windows 重叠I/O数据结构
    WSABUF          wsa_buf;        // 存储数据的缓冲区,用来给重叠操作传递参数.
    char            buffer[DEFAULT_CONN_BUFFER_SIZE];   // 对应 WSABUF 里的缓冲区
    EIOSocketState  io_type;        // 当前的I/O类型. IOCP没有像epoll那样的EPOLLIN,EPOLLOUT,只能通过自身携带的类型做标记.
};

/*
* 定义 AcceptEx 函数相关
*/
struct AcceptEx_t
{
    ACCEPTEX accept_ex_fn = nullptr;      // AcceptEx 函数指针
    SOCKET   socket_fd;         // 当前未决的客户端套接字  -AcceptEx
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
#endif
