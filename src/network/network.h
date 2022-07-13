#pragma once

#include "event.h"
#include "src/tools/ringbuffer.h"
#include "src/tools/memory_pool_lock_free.h"
#include "network_mgr.h"
#include "network_def.h"

namespace ToolBox
{

    class NetworkMaster;

    /// 事件队列
    using Event2Worker = RingBufferSPSC<NetEventWorker*, NETWORK_EVENT_QUEUE_MAX_COUNT>;
    /// 事件处理函数
    using EventHandler = std::function<void(Event* event)>;
    /*
    * Worker 网络基类
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
        virtual bool Init(NetworkMaster* master, NetworkType network_type);
        /*
        * 逆初始化网络
        */
        virtual bool UnInit();
        /*
        * 运行一次网络循环 派生类要调用基类的 Update()
        */
        virtual void Update();
        /*
        * @brief 在io多路复用中关闭监听socket
        * @param socket 的文件描述符
        */
        virtual void CloseListenInMultiplexing(int32_t socket_id) {};
        /*
        * 加入事件
        */
        void PushEvent(NetEventWorker* event);

    protected:
        /*
        * 主线程通知,工作线程内建立监听器
        */
        virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * 主线程通知,工作线程内建立连接器
        */
        virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * 主线程通知,关工作线程内闭网络连接
        */
        virtual void OnClose(uint64_t connect_id) = 0;
        /*
        * 主线程通知,工作线程内工作线程内发送
        */
        virtual void OnSend(uint64_t connect_id, const char* data, std::size_t size) = 0;

    public:
        /*
        * 工作线程内接收到新连接,通知主线程
        */
        void OnAccepted(uint64_t connect_id);
        /*
        * 工作线程内连接到远端成功,通知主线程
        */
        void OnConnected(uint64_t connect_id);
        /*
        * 工作线程内连接到远端失败,通知主线程
        */
        void OnConnectedFailed(ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内产生错误,通知主线程
        */
        void OnErrored(uint64_t connect_id, ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内关闭网络连接,通知主线程
        */
        void OnClosed(uint64_t connect_id, ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内接收到数据,通知主线程
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
    public:
        NetworkType GetNetworkType()
        {
            return network_type_;
        };

    private:
        NetworkType network_type_;          // 网络类型: TCP,UDP,KCP
        Event2Worker event2worker_;         // 主线程到工作线程的事件队列
        NetworkMaster* master_;             // 主线程中的网络管理器
    };

};  // ToolBox