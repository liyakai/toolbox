#pragma once
#include <atomic>
#include <cstdint>
#include <stdint.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include "network/network_api.h"
#include "network_def_internal.h"
#include "tools/ringbuffer.h"

namespace ToolBox
{
    class INetwork;
    class NetEventMain;
    class Event;
    class NetEventWorker;
    class EventDispatcher;

    /// 事件队列
    using Event2Main = RingBufferSPSC<NetEventMain*, NETWORK_EVENT_QUEUE_MAX_COUNT>;
    /// 事件处理函数
    using EventHandler = std::function<void(Event* event)>;
    /*
    * 定义网络主线程
    */
    class NetworkChannel
    {
    public:
        /*
        * 构造
        */
        NetworkChannel();
        /*
        * 析构
        */
        virtual ~NetworkChannel();
        /*
        * @brief 启动工作线程
        * @param net_thread_num 需要启动的网络线程数量
        */
        virtual bool Start(std::size_t net_thread_num = 1);
        /*
        * @brief 驱动主线程内事件处理
        */
        virtual void Update();
        /*
        * @brief 结束并等待工作线程结束,在主线程内调用
        */
        virtual void StopWait();
        /*
        * @brief 通知工作线程关闭连接
        * @param type 网络类型
        * @param conn_id 需要关闭连接的id
        */
        ENetErrCode Close(uint64_t conn_id);
        /*
        * @brief 通知工作线程发送数据
        * @param type 网络类型
        * @param conn_id 需要关闭连接的id
        * @param data 被发送数据的指针
        * @param conn_id 被发送数据的长度
        */
        ENetErrCode Send(uint64_t conn_id, const char* data, uint32_t size);
        /*
        * 通知网络线程建立一个监听器
        * @param type 网络类型
        * @param ip 监听ip
        * @param port 监听端口
        * @param send_buff_size 发送缓冲区大小
        * @param recv_buff_size 接收缓冲区大小
        */
        void Accept(NetworkType type, const std::string& ip, uint16_t port, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
        /*
        * 通知网络线程建立一个主动连接
        * @param type 网络类型
        * @param ip 连接ip
        * @param port 连接端口
        * @param send_buff_size 发送缓冲区大小
        * @param recv_buff_size 接收缓冲区大小
        */
        void Connect(NetworkType type, const std::string& ip, uint16_t port, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
        /*
        * @brief 网络线程投递事件到逻辑线程接口
        * @param event 事件
        * @return bool 是否成功
        */
        bool NotifyMain(NetEventMain* event);
    public:
        /*
        * @brief 网络库特性:网络线程模拟 Nagle 算法,减少系统调用,代价是在通信不够频繁的情况下可能会增加延迟.
        *        建议在服务器之间的内部等连接数较少的情况下使用,不建议在gate等与较多客户端连理连接的进程上使用.
        * @param packets_num 默认发送缓冲区超过10个包再进行发送.
        * @param timeout 默认超过2毫秒就进行发送.
        */
        void SetSimulateNagle(uint32_t packets_num = 10, uint32_t timeout = 2);
    public: // 回调函数方式的回调
        /*
        * @brief 设置绑定成功的回调
        */
        NetworkChannel& SetOnBinded(BindedMethod binded_method);
        /*
        * @brief 设置有新的连接事件的回调[尚未加入监听]
        */
        NetworkChannel& SetOnAccepting(AcceptingMethod accepting_method);
        /*
        * @brief 设置有新的连接事件的回调[已经加入监听]
        */
        NetworkChannel& SetOnAccepted(AcceptedMethod accepted_method);
        /*
        * @brief 设置主动连接成功的回调
        */
        NetworkChannel& SetOnConnected(ConnectedMethod connected_method);
        /*
        * @brief 设置主动连接失败的回调
        */
        NetworkChannel& SetOnConnectFailed(ConnectFailedMethod connect_failed_method);
        /*
        * @brief 设置发生错误的回调
        */
        NetworkChannel& SetOnErrored(ErroredMethod errored_method);
        /*
        * @brief 设置关闭连接的回调
        */
        NetworkChannel& SetOnClose(CloseMethod close_method);
        /*
        * @brief 设置接收的回调
        */
        NetworkChannel& SetOnReceived(ReceivedMethod received_method);

    protected:  // 继承方式的回调
        /*
        * 逻辑线程内处理监听端口绑定事件
        * @param conn_id 连接ID
        */
        virtual void OnBinded(NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port) {};
        /*
        * 逻辑线程内处理接受新连接事件[尚未加入监听]
        * @param conn_id 连接ID
        */
        virtual void OnAccepting(NetworkType type, int32_t fd) {};
        /*
        * 逻辑线程内处理接受新连接事件[已加入监听]
        * @param conn_id 连接ID
        */
        virtual void OnAccepted(NetworkType type, uint64_t conn_id) {};
        /*
        * 逻辑线程内处理主动连接事件
        * @param conn_id 连接ID
        */
        virtual void OnConnected(NetworkType type, uint64_t conn_id) {};
        /*
        * 逻辑线程内处理主动连接事件
        * @param conn_id 连接ID
        */
        virtual void OnConnectedFailed(NetworkType type, ENetErrCode err_code, int32_t err_no) {};
        /*
        * 逻辑线程内处理错误事件
        * @param conn_id 连接ID
        */
        virtual void OnErrored(NetworkType type, uint64_t conn_id, ENetErrCode err_code, int32_t err_no) {};
        /*
        * 逻辑线程内处理连接关闭事件
        * @param conn_id 连接ID
        */
        virtual void OnClose(NetworkType type, uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) {};
        /*
        * 逻辑线程内处理数据可读事件
        * @param conn_id 连接ID
        * @param data 内存指针
        * @param size 数据长度
        */
        virtual void OnReceived(NetworkType type, uint64_t conn_id, const char* data, size_t size) {};

    private:
        /*
        * 逻辑线程投递事件到网络线程
        * @param event 事件
        * @param type 网络类型
        */
        void NotifyWorker(NetEventWorker* event, NetworkType type, uint32_t net_thread_index);
        /*
        * 网络线程通知逻辑线程,监听端口已绑定.
        */
        void OnWorkerToMainBinded_(Event* event);
        /*
        * 网络线程通知逻辑线程,监听到新的连接,但尚未加入io多路复用.
        */
        void OnWorkerToMainAccepting_(Event* event);
        /*
        * 网络线程通知逻辑线程,新的连接已加入io多路复用.
        */
        void OnWorkerToMainAccepted_(Event* event);
        /*
        * 网络线程通知逻辑线程,连接已关闭
        */
        void OnWorkerToMainClose_(Event* event);
        /*
        * 网络线程通知逻辑线程,主动连接成功
        */
        void OnWorkerToMainConnected_(Event* event);
        /*
        * 网络线程通知逻辑线程,主动连接失败
        */
        void OnWorkerToMainConnectFailed_(Event* event);
        /*
        * 网络线程通知逻辑线程,发生错误
        */
        void OnWorkerToMainErrored_(Event* event);
        /*
        * 网络线程通知逻辑线程,收到消息
        */
        void OnWorkerToMainRecv_(Event* event);

        /*
        * 处理需要在逻辑线程处理的事件
        */
        void DispatchMainEvent_();
        /*
        * 根据类型获取网络实例
        * @param type 网络类型
        */
        INetwork* GetNetwork_(NetworkType type, uint32_t net_thread_index);
        /*
        * @brief 根据connid获取网络线程序号
        */
        uint32_t GetNetThreadIndex(uint64_t conn_id);

        /*
        * @brief 将文件描述符加入
        */
        void JoinIOMultiplexing(NetworkType type, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size);
    private:
        /*
        * @brief 设置系统可打开的最大文件描述符[需要root权限才可成功]
        */
        void SetSystemMaxOpenFiles();

    private:
        Event2Main event2main_;     // 主线程网络事件队列
        EventDispatcher* event_dispatcher_;  // 事件分发器
        std::mutex lock_;           // 多网络线程需要锁.
        std::atomic_bool stop_;     // 网络线程是否退出
        std::vector<std::unique_ptr<std::thread>> workers_;   // 工作线程,执行网络动作.
        using NetworkArray = std::vector<std::array<std::unique_ptr<INetwork>, NetworkType::NT_MAX>>;
        NetworkArray networks_;     // 网络实现
        std::unordered_map<uint64_t, NetworkType> conn_type_;   // conn_id 到 NetworkType的映射
    private:    // 回调函数
        BindedMethod binded_;           // 绑定的回调
        AcceptingMethod accepting_;    // 新连接事件[尚未加入监听]
        AcceptedMethod accepted_;     // 新连接事件[已经加入监听]
        ConnectedMethod connected_;     // 主动连接成功
        ConnectFailedMethod connect_failed_;  // 主动连接失败
        ErroredMethod errored_;         // 发生错误
        CloseMethod close_;             // 关闭事件的回调
        ReceivedMethod received_;       // 接收事件
    };

};  // ToolBox