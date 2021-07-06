#pragma once


constexpr size_t MAX_SOCKET_COUNT           =   40000;
constexpr size_t MAXEVENTS                  =   8192; /* epoll_create参数 */
constexpr size_t EPOLL_WAIT_MSECONDS        =   2;
constexpr size_t INVALID_CONN_ID	 	    =   UINT32_MAX;
constexpr size_t DEFAULT_RING_BUFF_SIZE     =   16*1024;     /* 16KB */
constexpr size_t MAX_RING_BUFF_SIZE         =   1024*1024;   /* 1MB */
constexpr size_t DEFAULT_BACKLOG_SIZE       =   256;    
constexpr size_t RECV_BUFFER_MAX_SIZE       =   1024 * 256;

/*
* 错误码
*/
enum class ErrCode
{
    ERR_SUCCESS = 0,
    ERR_MALLOC_FAILED, // 分配内存失败
    ERR_PARAM_NULLPTR, // 参数为空指针

};

enum class SocketType
{
    SOCK_TYPE_INVALID = 0,
    SOCK_TYPE_CONNECT,
    SOCK_TYPE_LISTEN,
};

enum class SocketState
{
    SOCK_STATE_INVALIED = 1,    // 初始状态
    SOCK_STATE_LISTENING = 2,   // 监听
    SOCK_STATE_CONNECTING = 3,  // 主动连接
    SOCK_STATE_ESTABLISHED = 4, // 连接建立
};

/*
* epoll 操作
*/
enum class EpollOperType
{
    EPOLL_OPER_ADD = 1,
    EPOLL_OPER_RDC = 2,
};

/*
* socket 可投递事件
*/
enum SockEventType
{
    SOCKET_EVENT_RECV = 1,
    SOCKET_EVENT_SEND = 2,
};
