#pragma once
#include "tools/object_pool.h"
#include "tools/object_pool_lock_free.h"
#include "tools/memory_pool.h"
#include "tools/memory_pool_lock_free.h"
#include "tools/log.h"

namespace ToolBox
{

    //     // 网络类型
    //     enum NetworkType
    //     {
    //         NT_UNKNOWN = 0,
    //         NT_TCP,
    //         NT_UDP,
    //         NT_KCP,
    //         NT_MAX,
    //     };

    //     enum class ENetErrCode
    //     {
    //         NET_NO_ERROR = 0,
    //         NET_SYS_ERROR,            // 系统错误，同时会返回errno
    //         NET_INVALID_NETWORK_TYPE, // 发送非法的网络类型.合法范围见 NetworkType
    //         NET_INVALID_NET_THREAD_INDEX, // 发送非法的网络线程序号.序号超出了实际线程数量
    //         NET_INVALID_PACKET_SIZE,  // 错误的包长
    //         NET_CONNECT_FAILED,       // 连接出错
    //         NET_LISTEN_FAILED,        // 监听出错
    //         NET_ACCEPT_FAILED,        // accept 出错
    //         NET_SEND_FAILED,          // 发包出错
    //         NET_RECV_FAILED,          // 收包出错
    //         NET_ALLOC_FAILED,         // 申请内存出错，在socket对象申请上
    //         NET_SEND_BUFF_OVERFLOW,   // 发送缓冲区满
    //         NET_RECV_BUFF_OVERFLOW,   // 接收缓冲区满
    //         NET_ENCODE_BUFF_OVERFLOW, // 打包缓冲区满
    //         NET_DECODE_BUFF_OVERFLOW, // 解包缓冲区满
    //         NET_INVALID_EVENT,        // event 无效

    //         NET_SEND_PIPE_OVERFLOW, // 发送ringbuffer满
    //         NET_RECV_PIPE_OVERFLOW, // 接收ringbuffer满
    //     };

    //     // 主线程与网络线程之间的队列的最大数量
    //     constexpr std::size_t NETWORK_EVENT_QUEUE_MAX_COUNT = 32 * 1024;

    //     // 定义 网络库日志接口
    // #define NetworkLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
    // #define NetworkLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
    // #define NetworkLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
    // #define NetworkLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
    // #define NetworkLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
    // #define NetworkLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)

    /************************************************************
    **********     网络库获取对象的三种方法       ****************
    ************************************************************/

    // 宏定义获取对象的方法[调用入口]
#define GET_NET_OBJECT(OBJECT_TYPE, ...) \
    GET_NET_OBJECT_RAW(OBJECT_TYPE, __VA_ARGS__)
    // 宏定义释放对象的方法[调用入口]
#define GIVE_BACK_OBJECT(POINTER)   \
    GIVE_BACK_OBJECT_RAW(POINTER)

    //-----------[下面是三种内部实现选项]-----------------

    // 宏定义获取原生网络事件对象[选项1]
#define GET_NET_OBJECT_RAW(OBJECT_TYPE, ...) \
    new OBJECT_TYPE(__VA_ARGS__)
    // 宏定义释放原生网络事件对象[选项1]
#define GIVE_BACK_OBJECT_RAW(POINTER)   \
    delete POINTER;


    // 宏定义从对象池中获取网络事件对象[选项2]
#define GET_NET_OBJECT_OP(OBJECT_TYPE, ...) \
    GetObject<OBJECT_TYPE>(__VA_ARGS__)
    // 宏定义释放对象到对象池中[选项2]
#define GIVE_BACK_OBJECT_OP(POINTER)    \
    GiveBackObject(POINTER);

    // 宏定义从无锁对象池中获取网络事件对象[选项3]
#define GET_NET_OBJECT_OPLF(OBJECT_TYPE, ...) \
    GetObjectLockFree<OBJECT_TYPE>(__VA_ARGS__)
    // 宏定义释放对象到无锁对象池中[选项3]
#define GIVE_BACK_OBJECT_OPLF(POINTER)  \
    GiveBackObjectLockFree(POINTER);

    /************************************************************
    **********     网络库获取内存的三种方法       ****************
    ************************************************************/

    // 宏定义获取对象的方法[调用入口]
#define GET_NET_MEMORY(SIZE) \
    GET_NET_MEMORY_RAW(SIZE)
    // 宏定义释放对象的方法[调用入口]
#define GIVE_BACK_MEMORY(POINTER, ...)   \
    GIVE_BACK_MEMORY_RAW(POINTER, __VA_ARGS__)

    //-----------[下面是三种内部实现选项]-----------------

    // 宏定义获取原生内存[选项1]
#define GET_NET_MEMORY_RAW(SIZE) \
    new char[SIZE]
    // 宏定义释放原生内存[选项1]
#define GIVE_BACK_MEMORY_RAW(POINTER,...)   \
    delete POINTER;


    // 宏定义从内存池中获取内存[选项2]
#define GET_NET_MEMORY_MP(SIZE) \
    MemPoolMgr->GetMemory(SIZE);
    // 宏定义释放内存到内存池中[选项2]
#define GIVE_BACK_MEMORY_MP(POINTER,...)    \
    MemPoolMgr->GiveBack((char*)POINTER, __VA_ARGS__);

    // 宏定义从无锁对象池中获取网络事件对象[选项3]
#define GET_NET_OBJECT_MPLF(SIZE) \
    MemPoolLockFreeMgr->GetMemory(SIZE);
    // 宏定义释放对象到无锁对象池中[选项3]
#define GIVE_BACK_OBJECT_MPLF(POINTER,...)  \
    MemPoolLockFreeMgr->GiveBack((char*)POINTER, __VA_ARGS__);

};  // ToolBox

#include "network/network_def.h"