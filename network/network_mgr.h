#pragma once
#include <atomic>
#include <thread>
#include "tools/ringbuffer.h"
#include "event.h"
#include "network.h"

// 网络类型
enum NetworkType
{
    Unknown = 0,
    NT_TCP,
    NT_MAX,
};

using Event2Main = RingBuffer<NetEventMain*, 1024>;

class NetworkMaster
{
public:
    /*
    * 构造
    */
    NetworkMaster();
private:
    Event2Main event2main_;     // 主线程网络事件队列
    std::atomic_bool stop_;     // 县城退出
    std::unique_ptr<std::thread> worker_;   // 工作线程
    using NetworkArray = std::array<std::unique_ptr<INetwork>, NT_MAX>;
    NetworkArray networks_;     // 网络实现 
};

