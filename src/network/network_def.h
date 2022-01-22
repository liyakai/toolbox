#pragma once
#include "src/tools/object_pool.h"
#include "src/tools/object_pool_lock_free.h"

namespace ToolBox{

// 网络类型
enum NetworkType
{
    NT_UNKNOWN = 0,
    NT_TCP,
    NT_UDP,
    NT_KCP,
    NT_MAX,
};

enum class ENetErrCode
{
    NET_NO_ERROR = 0,
    NET_SYS_ERROR,            // 系统错误，同时会返回errno
    NET_INVALID_NETWORK_TYPE, // 发送非法的网络类型.合法范围见 NetworkType
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
    NET_INVALID_SOCKET,       // socket 无效

    NET_SEND_PIPE_OVERFLOW, // 发送ringbuffer满
    NET_RECV_PIPE_OVERFLOW, // 接收ringbuffer满
};

// 主线程与网络线程之间的队列的最大数量
constexpr std::size_t NETWORK_EVENT_QUEUE_MAX_COUNT = 32 * 1024; 


/************************************************************
**********     获取对象的三种方法      **********************
************************************************************/

// 宏定义获取对象的方法[调用入口]
#define GET_NET_OBJECT(OBJECT_TYPE, ...) \
    GET_NET_OBJECT_OELF(OBJECT_TYPE, __VA_ARGS__)
// 宏定义释放对象的方法[调用入口]
#define GIVE_BACK_OBJECT(POINTER)   \
    GIVE_BACK_OBJECT_OELF(POINTER)

//-----------[下面是三种内部实现选项]-----------------

// 宏定义获取原生网络事件对象[选项1]
#define GET_NET_OBJECT_RAW(OBJECT_TYPE, ...) \
    new OBJECT_TYPE(__VA_ARGS__)
// 宏定义释放原生网络事件对象[选项1]
#define GIVE_BACK_OBJECT_RAW(POINTER)   \
    delete POINTER;


// 宏定义从对象池中获取网络事件对象[选项2]
#define GET_NET_OBJECT_OE(OBJECT_TYPE, ...) \
    GetObject<OBJECT_TYPE>(__VA_ARGS__)
// 宏定义释放对象到对象池中[选项2]
#define GIVE_BACK_OBJECT_OE(POINTER)    \
    GiveBackObject(POINTER);

// 宏定义从无锁对象池中获取网络事件对象[选项3]
#define GET_NET_OBJECT_OELF(OBJECT_TYPE, ...) \
    GetObjectLockFree<OBJECT_TYPE>(__VA_ARGS__)
// 宏定义释放对象到无锁对象池中[选项3]
#define GIVE_BACK_OBJECT_OELF(POINTER)  \
    GiveBackObjectLockFree(POINTER);

};  // ToolBox