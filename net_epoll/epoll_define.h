#pragma once

#define MAX_SOCKET_COUNT 20000
#define MAXEVENTS 8192 /* epoll_create参数 */
#define EPOLL_WAIT_MSECONDS 2

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
    SOCK_STATE_INVALIE = 0,
    SOCK_STATE_LISTENING,
    SOCK_STATE_CONNECTING,
    SOCK_STATE_ESTABLISHED,
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

struct EpollData
{
    SocketType socket_type; // socket type
    void *ptr;              // socket ptr
    EpollData()
        : socket_type(SocketType::SOCK_TYPE_INVALID), ptr(nullptr)
    {
    }
    void Reset()
    {
        socket_type = SocketType::SOCK_TYPE_INVALID;
        ptr = nullptr;
    }
};