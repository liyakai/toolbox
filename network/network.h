#pragma once

#include "event.h"
#include "tools/ringbuffer.h"
#include "tools/memory_pool.h"

using Event2Worker = RingBuffer<NetEventWorker*, 1024>;
/*
* 网络接口
*/
class INetwork
{
public:
    /*
    * 构造
    */
    INetwork();
    /*
    * 析构
    */
    virtual ~INetwork()
    {
        while (!event2worker_.Empty())
        {
            NetEventWorker* event;
            if(event2worker_.Read<NetEventWorker*>(event))
            {
                MemPoolMgr->GiveBack((char*)event);
            }
            
        }
        
    }
    /*
    * 初始化网络
    */
    virtual void Init();
    /*
    * 运行一次网络循环
    */
    virtual void Update() = 0;

protected:
    /*
    * 建立监听器
    */
    virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port) = 0;
    /*
    * 建立连接器
    */
    virtual uint64_t OnNewConnector(const std::string& ip, const uint16_t port) = 0;
    /*
    * 关闭网络连接
    */
    virtual void OnClose(uint64_t connect_id) = 0;
    /*
    * 发送
    */
    virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size);
    /*
    * 接收到新连接
    */
    void OnAccepted(uint64_t connect_id);
    /*
    * 连接到远端
    */
    void OnConnected(uint64_t connect_id);
    /*
    * 关闭网络连接
    */
    void OnClosed(uint64_t connect_id);
    /*
    * 接收到数据
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
private:
    Event2Worker event2worker_;

};

