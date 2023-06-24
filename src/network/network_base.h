#pragma once

#include "tools/ringbuffer.h"
#include "tools/memory_pool_lock_free.h"
#include "network_channel.h"
#include "network_def_internal.h"
#include <cstdint>
#include <functional>
#include <stdint.h>

namespace ToolBox
{

    class NetworkChannel;
    class EventDispatcher;
    class NetEventWorker;
    class Event;
    /// 事件队列
    using Event2Worker = RingBufferSPSC<NetEventWorker*, NETWORK_EVENT_QUEUE_MAX_COUNT>;
    /// 事件处理函数
    using EventHandler = std::function<void(Event* event)>;
    /*
    * Worker 网络基类
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
        virtual ~INetwork();
        /*
        * 初始化网络
        */
        virtual bool Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index);
        /*
        * 逆初始化网络
        */
        virtual bool UnInit();
        /*
        * 运行一次网络循环 派生类要调用基类的 Update()
        */
        virtual void Update(std::time_t time_stamp);
        /*
        * @brief 在io多路复用中关闭监听socket
        * @param socket 的文件描述符
        */
        virtual void CloseListenInMultiplexing(int32_t socket_id) {};
        /*
        * 加入事件
        */
        void PushEvent(NetEventWorker* event);

    public:
        /*
        * 工作线程内接收到新连接,通知主线程[尚未加入监听]

        */
        void OnAccepting(uint64_t opaque, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
        /*
        * 工作线程内接收到新连接,通知主线程[已加入监听]
        */
        void OnAccepted(uint64_t opaque, uint64_t connect_id);
        /*
        * 工作线程内连接到远端成功,通知主线程
        */
        void OnConnected(uint64_t opaque, uint64_t connect_id);
        /*
        * 工作线程内连接到远端失败,通知主线程
        */
        void OnConnectedFailed(uint64_t opaque, ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内产生错误,通知主线程
        */
        void OnErrored(uint64_t opaque, uint64_t connect_id, ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内关闭网络连接,通知主线程
        */
        void OnClosed(uint64_t opaque, uint64_t connect_id, ENetErrCode err_code, int32_t err_no);
        /*
        * 工作线程内接收到数据,通知主线程
        */
        void OnReceived(uint64_t opaque, uint64_t connect_id, const char* data, uint32_t size);
    public:
        /*
        * @brief 获取网络库特性参数->最大累计包数.网络线程模拟 Nagle 算法,减少系统调用,代价是在通信不够频繁的情况下可能会增加延迟.
        */
        int32_t GetSimulateNaglePacketsNum();
        /*
        * @brief 获取网络库特性参数->超时时间,单位毫秒(ms).网络线程模拟 Nagle 算法,减少系统调用,代价是在通信不够频繁的情况下可能会增加延迟.
        */
        int32_t GetSimulateNagleTimeout();



    protected:
        /*
        * 主线程通知,工作线程内建立监听器
        */
        virtual uint64_t OnNewAccepter(uint64_t opaque, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * 主线程通知,将fd加入io多路复用
        */
        virtual uint64_t OnJoinIOMultiplexing(uint64_t opaque, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * 主线程通知,工作线程内建立连接器
        */
        virtual uint64_t OnNewConnecter(uint64_t opaque, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * 主线程通知,关工作线程内闭网络连接
        */
        virtual void OnClose(uint64_t connect_id) = 0;
        /*
        * 主线程通知,工作线程内工作线程内发送
        */
        virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size) = 0;

    protected:
        /*
        * 时间函数,统一更新,减少系统调用
        */
        std::time_t GetNetTime() const
        {
            return update_timestamp_;
        }

    private:
        /*
        * 通知网络线程建立监听器
        */
        void OnMainToWorkerNewAccepter_(Event* event);
        /*
        * 通知网络线程加入io多路复用
        */
        void OnMainToWorkerJoinIOMultiplexing_(Event* event);
        /*
        * 通知网络线程建立连接器
        */
        void OnMainToWorkerNewConnecter_(Event* event);
        /*
        * 通知网络线程关闭网络连接
        */
        void OnMainToWorkerClose_(Event* event);
        /*
        * 通知网络线程发送消息
        */
        void OnMainToWorkerSend_(Event* event);
        /*
        * 通知网络线程设置模拟Nagle算法
        */
        void SetSimulateNagle_(Event* event);
        /*
        * 处理需要在网络线程中处理的事件
        */
        void HandleEvents_();

    public:
        /*
        * 获取网络通信类型.
        */
        NetworkType GetNetworkType()
        {
            return network_type_;
        };
        /*
        * 获取网络线程序号
        */
        uint32_t GetThreadIndex();

    private:
        NetworkType network_type_;          // 网络类型: TCP,UDP,KCP
        Event2Worker event2worker_;         // 主线程到工作线程的事件队列
        EventDispatcher* event_dispatcher_;  // 事件分发器
        NetworkChannel* master_;            // 主线程中的网络管理器
        uint32_t net_thread_index_ = 0;     // 网络线程序号
        std::time_t update_timestamp_ = 0;  // 由Update更新的时间
        int32_t nagle_packets_num_ = -1;    // 模拟Nagle 参数,累计 packets_num_ 包后再进行发送操作.
        int32_t nagle_timeout_ = -1;        // 模拟Nagle 参数,timeout_ 后触发发送操作.单位毫秒(ms)
    };

};  // ToolBox