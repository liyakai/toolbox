#pragma once

#include "event.h"
#include "src/tools/ringbuffer.h"
#include "src/tools/memory_pool.h"
#include "network_mgr.h"

class NetworkMaster;
/// 事件队列
using Event2Worker = RingBuffer<NetEventWorker*, 1024>;
/// 事件处理函数
using EventHandler = std::function<void(Event* event)>;
/*
* 网络基类
*/
class INetwork : public EventBasedObject
{
public:
    /*
    * 构造
    */
    INetwork();
    /*
    * 析构
    */
    virtual ~INetwork();
    /*
    * 初始化网络
    */
    virtual void Init(NetworkMaster* master);
    /*
    * 逆初始化网络
    */
    virtual void UnInit();
    /*
    * 运行一次网络循环
    */
    virtual void Update();
    /*
    * 加入事件
    */
    void PushEvent(NetEventWorker* event);

protected:
    /*
    * 工作线程内建立监听器
    */
    virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port) = 0;
    /*
    * 工作线程内建立连接器
    */
    virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port) = 0;
    /*
    * 关工作线程内闭网络连接
    */
    virtual void OnClose(uint64_t connect_id) = 0;
    /*
    * 工作线程内工作线程内发送
    */
    virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size) = 0;
    /*
    * 工作线程内接收到新连接
    */
    void OnAccepted(uint64_t connect_id);
    /*
    * 工作线程内连接到远端
    */
    void OnConnected(uint64_t connect_id);
    /*
    * 工作线程内关闭网络连接
    */
    void OnClosed(uint64_t connect_id);
    /*
    * 工作线程内接收到数据
    */
    void OnReceived(uint64_t connect_id, const char* data, uint32_t size);
private:
    /*
    * 通知工作线程建立监听器
    */
    void OnMainToWorkerNewAccepter_(Event* event);
    /*
    * 通知工作线程建立连接器
    */
    void OnMainToWorkerNewConnecter_(Event* event);
    /*
    * 通知工作线程关闭网络连接
    */
    void OnMainToWorkerClose_(Event* event);
    /*
    * 通知工作线程发送消息
    */
    void OnMainToWorkerSend_(Event* event);
    /*
    * 处理需要在工作线程中处理的事件
    */
    void HandleEvents_();
private:
    Event2Worker event2worker_;
    NetworkMaster* master_;
};

