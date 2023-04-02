
#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include "network/network_def.h"

namespace ToolBox
{

    using BindedMethod = std::function<void(NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port)>;
    using AcceptingMethod = std::function<void(NetworkType type, int32_t fd)>;
    using AcceptedMethod = std::function<void(NetworkType type, uint64_t conn_id)>;
    using ConnectedMethod = std::function<void(NetworkType type, uint64_t conn_id)>;
    using ConnectFailedMethod = std::function<void(NetworkType type, ENetErrCode err_code, int32_t err_no)>;
    using ErroredMethod = std::function<void(NetworkType type, uint64_t conn_id, ENetErrCode err_code, int32_t err_no)>;
    using CloseMethod = std::function<void(NetworkType type, uint64_t conn_id, ENetErrCode net_err, int32_t sys_err)>;
    using ReceivedMethod = std::function<void(NetworkType type, uint64_t conn_id, const char* data, size_t size)>;

    class NetworkChannel;
    class Network
    {
    public:
        /*
        * 构造
        */
        Network();
        /*
        * 析构
        */
        virtual ~Network();
        /*
        * @brief 启动工作线程
        * @param net_thread_num 需要启动的网络线程数量
        */
        bool Start(std::size_t net_thread_num = 1);
        /*
        * @brief 驱动主线程内事件处理
        */
        void Update();
        /*
        * @brief 结束并等待工作线程结束,在主线程内调用
        */
        void StopWait();
        /*
        * @brief 通知工作线程关闭连接
        * @param type 网络类型
        * @param conn_id 需要关闭连接的id
        */
        void Close(NetworkType type, uint64_t conn_id);
        /*
        * @brief 通知工作线程发送数据
        * @param type 网络类型
        * @param conn_id 需要关闭连接的id
        * @param data 被发送数据的指针
        * @param conn_id 被发送数据的长度
        */
        void Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size);
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
    public:
        /*
        * @brief 网络库特性:网络线程模拟 Nagle 算法,减少系统调用,代价是在通信不够频繁的情况下可能会增加延迟.
        *        建议在服务器之间的内部等连接数较少的情况下使用,不建议在gate等与较多客户端连理连接的进程上使用.
        * @param packets_num 默认发送缓冲区超过10个包再进行发送.
        * @param timeout 默认超过2毫秒就进行发送.
        */
        void SetSimulateNagle(uint32_t packets_num = 10, uint32_t timeout = 2);
    public:
        /*
        * @brief 设置绑定成功的回调
        */
        Network& SetOnBinded(BindedMethod binded_method);
        /*
        * @brief 设置有新的连接事件的回调[尚未加入监听]
        */
        Network& SetOnAccepting(AcceptingMethod accepting_method);
        /*
        * @brief 设置有新的连接事件的回调[已经加入监听]
        */
        Network& SetOnAccepted(AcceptedMethod accepted_method);
        /*
        * @brief 设置主动连接成功的回调
        */
        Network& SetOnConnected(ConnectedMethod connected_method);
        /*
        * @brief 设置主动连接失败的回调
        */
        Network& SetOnConnectFailed(ConnectFailedMethod connect_failed_method);
        /*
        * @brief 设置发生错误的回调
        */
        Network& SetOnErrored(ErroredMethod errored_method);
        /*
        * @brief 设置关闭连接的回调
        */
        Network& SetOnClose(CloseMethod close_method);
        /*
        * @brief 设置接收的回调
        */
        Network& SetOnReceived(ReceivedMethod receive_method);
    private:
        NetworkChannel* network_channel_ = nullptr;
    };

};  // ToolBox