#pragma once
#include <cstdint>
#include <cstddef>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

//#include <windows.h>
#include <cstddef>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "Mswsock.h"

#elif defined(__linux__)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/event.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif  // #if (defined(WIN32) || defined(_WIN64))

namespace ToolBox{

constexpr std::size_t MAX_SOCKET_COUNT = 40000;
constexpr std::size_t INVALID_CONN_ID = UINT32_MAX;
constexpr std::size_t DEFAULT_CONN_BUFFER_SIZE = 256 * 1024;        /* 256 k */
constexpr std::size_t DEFAULT_RING_BUFF_SIZE = 256 * 1024;        /* 256 k */
constexpr std::size_t DEFAULT_BACKLOG_SIZE = 256;
// 
constexpr int32_t KCP_TRANSPORT_MTU = 1000;
constexpr uint32_t KCP_CONV = 0x01020304;          // kcp会话ID, must equal in two endpoint from the same connection

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
* socket的状态
*/
enum SocketState
{
    SOCK_STATE_INVALIED     = 1 << 0,   // 初始状态
    SOCK_STATE_LISTENING    = 1 << 1,   // 监听
    SOCK_STATE_CONNECTING   = 1 << 2,   // 主动连接
    SOCK_STATE_ESTABLISHED  = 1 << 3,   // 连接建立
    SOCK_STATE_RECV         = 1 << 4,   // 接收[iocp中persocket的细分状态]
    SOCK_STATE_SEND         = 1 << 5,   // 发送[iocp中persocket的细分状态]
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

};  // ToolBox

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "src/network/net_imp/net_iocp/iocp_define.h"
#elif defined(__linux__)
#include "src/network/net_imp/net_epoll/epoll_define.h"
#elif defined(__APPLE__)
#include "src/network/net_imp/net_kqueue/kqueue_define.h"
#endif
