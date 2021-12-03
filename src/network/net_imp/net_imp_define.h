#pragma once
#include <cstdint>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//#include <windows.h>
#include <cstddef>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "Mswsock.h"

constexpr std::size_t INVALID_SOCKET = -1;

#elif defined(__linux__)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#endif  // #if (defined(WIN32) || defined(_WIN64))

constexpr std::size_t MAX_SOCKET_COUNT = 40000;
constexpr std::size_t INVALID_CONN_ID = UINT32_MAX;
constexpr std::size_t DEFAULT_CONN_BUFFER_SIZE = 256 * 1024;        /* 256 k */
constexpr std::size_t DEFAULT_RING_BUFF_SIZE = 256 * 1024;        /* 256 k */
constexpr std::size_t DEFAULT_BACKLOG_SIZE = 256;
// 
constexpr int32_t KCP_TRANSPORT_MTU = 1000;
constexpr uint32_t KCP_CONV = 0x01020304;          // kcp会话ID

using SocketAddress = struct sockaddr_in;
/*
* 错误码
*/
enum class ErrCode
{
    ERR_SUCCESS = 0,
    ERR_MALLOC_FAILED, // 分配内存失败
    ERR_PARAM_NULLPTR, // 参数为空指针
    ERR_INSUFFICIENT_LENGTH,    // 长度不足
    ERR_INVALID_PACKET_SIZE,    // 包长度非法

};

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
* epoll下socket的状态
*/
enum class SocketState
{
    SOCK_STATE_INVALIED = 1,    // 初始状态
    SOCK_STATE_LISTENING = 2,   // 监听
    SOCK_STATE_CONNECTING = 3,  // 主动连接
    SOCK_STATE_ESTABLISHED = 4, // 连接建立
};

/*
* Event 操作
*/
enum class EventOperType
{
    EVENT_OPER_ADD = 1,
    EVENT_OPER_RDC = 2,
};

/*
* socket 可投递事件
*/
enum SockEventType
{
    SOCKET_EVENT_INVALID = 0,
    SOCKET_EVENT_RECV = 1,
    SOCKET_EVENT_SEND = 2,
    SOCKET_EVENT_ERR = 4,
};

/*
* UDP socket 类型
*/
enum class UdpType
{
    UNKNOWN = 0,
    ACCEPTOR = 1,
    CONNECTOR = 2,
    REMOTE = 3,
};


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "src/network/net_imp/net_iocp/iocp_define.h"
#elif defined(__linux__)
#include "net_epoll/epoll_define.h"

constexpr std::size_t INVALID_SOCKET = -1;

#endif  // #if (defined(WIN32) || defined(_WIN64))