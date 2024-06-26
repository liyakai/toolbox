#pragma once

#include <cstdint>
namespace ToolBox
{

    // 网络类型
    enum NetworkType
    {
        NT_UNKNOWN = 0,
        NT_TCP,
        NT_UDP,
        NT_KCP,
        NT_MAX,
    };

    enum class ENetErrCode : int32_t
    {
        NET_SUCCESS = 0,
        NET_SYS_ERROR,            // 系统错误，同时会返回errno
        NET_INVALID_NETWORK_TYPE, // 发送非法的网络类型.合法范围见 NetworkType
        NET_INVALID_NET_THREAD_INDEX, // 发送非法的网络线程序号.序号超出了实际线程数量
        NET_INVALID_PACKET_SIZE,  // 错误的包长
        NET_CONNECT_FAILED,       // 连接出错
        NET_LISTEN_FAILED,        // 监听出错
        NET_ACCEPT_FAILED,        // accept 出错
        NET_SEND_FAILED,          // 发包出错
        NET_RECV_FAILED,          // 收包出错
        NET_ALLOC_FAILED,         // 申请内存出错，在socket对象申请上
        NET_SEND_BUFF_OVERFLOW,   // 发送缓冲区满
        NET_RECV_BUFF_OVERFLOW,   // 接收缓冲区满
        NET_ENCODE_BUFF_OVERFLOW, // 打包缓冲区满
        NET_DECODE_BUFF_OVERFLOW, // 解包缓冲区满
        NET_INVALID_EVENT,        // event 无效
        NET_SEND_PIPE_OVERFLOW,   // 发送ringbuffer满
        NET_RECV_PIPE_OVERFLOW,   // 接收ringbuffer满
        NET_INVALID_CONNID,       // 无效的连接ID
        NET_CACHED_EVENT_OVERFLOW,// 逻辑线程缓存的EVENT数量溢出
    };

    // 主线程与网络线程之间的队列的最大数量
    constexpr std::size_t NETWORK_EVENT_QUEUE_MAX_COUNT = 32 * 1024;
};  // ToolBox