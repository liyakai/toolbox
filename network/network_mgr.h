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
    /*
    * 析构
    */
    virtual ~NetworkMaster();
    /*
    * 启动工作线程
    */
    virtual bool Start();
    /*
    * 驱动主线程内事件处理
    */
    virtual void Update();
    /*
    * 结束并等待工作线程结束,在祝县城内调用
    */
    virtual void StopWait();
    /*
    * 通知工作线程关闭连接
    */
    void Close(NetworkType type, uint64_t conn_id);
    /*
    * 通知工作线程发送数据
    */
    void Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size);


private:
    Event2Main event2main_;     // 主线程网络事件队列
    std::atomic_bool stop_;     // 县城退出
    std::unique_ptr<std::thread> worker_ = nullptr;   // 工作线程
    using NetworkArray = std::array<std::unique_ptr<INetwork>, NT_MAX>;
    NetworkArray networks_;     // 网络实现 
};

