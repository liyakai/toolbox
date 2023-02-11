#pragma once
#include <atomic>
#include <cstdint>
#include <stdint.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include "network_def.h"
#include "tools/ringbuffer.h"
#include "event.h"
#include "network.h"

namespace ToolBox
{
    class INetwork;

    /// 事件队列
    using Event2Main = RingBufferSPSC<NetEventMain*, NETWORK_EVENT_QUEUE_MAX_COUNT>;
    /// 事件处理函数
    using EventHandler = std::function<void(Event* event)>;
    /*
    * 定义网络主线程
    */
    class NetworkChannel : public EventBasedObject
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
        * 启动工作线程
        */
        virtual bool Start(std::size_t net_thread_num = 1);
        /*
        * 驱动主线程内事件处理
        */
        virtual void Update();
        /*
        * 结束并等待工作线程结束,在主线程内调用
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
        /*
        * 工作线程投递事件到主线程接口
        * @param event 事件
        * @return bool 是否成功
        */
        bool NotifyMain(NetEventMain* event);
    public:
        /*
        * 通知工作线程建立一个监听器
        * @param ip 监听ip
        * @param port 监听端口
        * @param type 网络类型
        * @param send_buff_size 发送缓冲区大小
        * @param recv_buff_size 接收缓冲区大小
        */
        void Accept(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
        /*
        * 通知工作线程建立一个主动连接
        * @param ip 连接ip
        * @param port 连接端口
        * @param type 网络类型
        * @param send_buff_size 发送缓冲区大小
        * @param recv_buff_size 接收缓冲区大小
        */
        void Connect(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
        /*
        * 主线程投递事件到工作线程
        * @param event 事件
        * @param type 网络类型
        */
        void NotifyWorker(NetEventWorker* event, NetworkType type, uint32_t net_thread_index);
    protected:
        /*
        * 主线程内处理接受新连接事件[尚未加入监听]
        * @param conn_id 连接ID
        */
        virtual void OnAcceptting(NetworkType type, int32_t fd) {};
        /*
        * 主线程内处理接受新连接事件[已加入监听]
        * @param conn_id 连接ID
        */
        virtual void OnAccepted(NetworkType type, uint64_t conn_id) {};
        /*
        * 主线程内处理主动连接事件
        * @param conn_id 连接ID
        */
        virtual void OnConnected(NetworkType type, uint64_t conn_id) {};
        /*
        * 主线程内处理主动连接事件
        * @param conn_id 连接ID
        */
        virtual void OnConnectedFailed(NetworkType type, ENetErrCode err_code, int32_t err_no) {};
        /*
        * 主线程内处理错误事件
        * @param conn_id 连接ID
        */
        virtual void OnErrored(NetworkType type, uint64_t conn_id, ENetErrCode err_code, int32_t err_no) {};
        /*
        * 主线程内处理连接关闭事件
        * @param conn_id 连接ID
        */
        virtual void OnClose(NetworkType type, uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) {};
        /*
        * 主线程内处理数据可读事件
        * @param conn_id 连接ID
        * @param data 内存指针
        * @param size 数据长度
        */
        virtual void OnReceived(NetworkType type, uint64_t conn_id, const char* data, size_t size) {};

    private:
        /*
        * 工作线程通知主线程,监听到新的连接,但尚未加入io多路复用.
        */
        void OnWorkerToMainAcceptting_(Event* event);
        /*
        * 工作线程通知主线程,新的连接已加入io多路复用.
        */
        void OnWorkerToMainAccepted_(Event* event);
        /*
        * 工作线程通知主线程,连接已关闭
        */
        void OnWorkerToMainClose_(Event* event);
        /*
        * 工作线程通知主线程,主动连接成功
        */
        void OnWorkerToMainConnected_(Event* event);
        /*
        * 工作线程通知主线程,主动连接失败
        */
        void OnWorkerToMainConnectFailed_(Event* event);
        /*
        * 工作线程通知主线程,发生错误
        */
        void OnWorkerToMainErrored_(Event* event);
        /*
        * 工作线程通知主线程,收到消息
        */
        void OnWorkerToMainRecv_(Event* event);

        /*
        * 处理需要在主线程处理的事件
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
        std::mutex lock_;           // 多网络线程需要锁.
        std::atomic_bool stop_;     // 网络线程是否退出
        std::vector<std::unique_ptr<std::thread>> workers_;   // 工作线程,执行网络动作.
        using NetworkArray = std::vector<std::array<std::unique_ptr<INetwork>, NetworkType::NT_MAX>>;
        NetworkArray networks_;     // 网络实现
    };

};  // ToolBox

