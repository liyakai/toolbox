#pragma once
#include "net_imp_define.h"
#include "../network_base.h"
#include <cstdint>

namespace ToolBox
{

    /*
    * 定义 socket 基类
    */
    class BaseSocket
    {
    public:
        /*
        * 构造
        */
        BaseSocket();
        /*
        * 析构
        */
        virtual ~BaseSocket();
        /*
        * 重置
        */
        virtual void Reset();
        /*
        * @brief 设置 tcp_network
        * @param network 工作线程
        */
        void SetNetwork(INetwork* network)
        {
            p_network_ = network;
        }
        /*
        * 获取连接ID
        */
        uint32_t GetConnID()
        {
            return conn_id_;
        }
        /*
        * 设置 分配的连接ID
        */
        void SetConnID(uint32_t id)
        {
            conn_id_ = id;
        }
        /*
        * 获取信道标记
        */
        uint32_t GetOpaque()
        {
            return opaque_;
        }
        /*
        * 设置 分配的连接ID
        */
        void SetOpaque(uint64_t opaque)
        {
            opaque_ = opaque;
        }
        /*
        * 更新事件
        * @params event_type 事件类型
        * @params ts 时间戳
        */
        virtual void UpdateEvent(SockEventType event_type, time_t ts) = 0;
        /*
        * @brief 监听(模拟)
        * @param ip 地址
        * @param port 端口
        */
        virtual bool InitNewAccepter(uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;

        /*
        *  初始化从accpet函数接收得来的socket
        */
        virtual bool InitAccpetSocket(uint64_t opaque, int32_t socket_fd, std::string ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * @brief 连接
        * @param ip 地址
        * @param port 端口
        * @return 是否成功
        */
        virtual bool InitNewConnecter(uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
        /*
        * @brief 发送 [原生UDP发送接口,不适用于kcp等协议的发送入口]
        * @param buffer 数据指针
        * @param length 数据长度
        * @param address 目标地址
        */
        virtual void Send(const char* buffer, std::size_t length) = 0;
        /*
        * Update
        */
        virtual void Update(std::time_t time_stamp) {};
        /*
        * 获取socket状态
        */
        virtual SocketState GetSocketState()
        {
            return SOCK_STATE_INVALIED;
        }
        /*
        * 获取事件类型
        * @return 可投递事件类型
        */
        int32_t GetEventType() const
        {
            return event_type_;
        };
        /*
        * 设置可投递类型
        */
        void SetSockEventType(int32_t type)
        {
            event_type_ = type;
        }
        /*
        * 发送错误,向主线程报告
        */
        void OnErrored(ENetErrCode err_code, int32_t err_no);
        /*
        * 重置接收 AsyncSocket
        */
        virtual void ResetRecvAsyncSocket() {};
        /*
        * 重置发送 AsyncSocket
        */
        virtual void ResetSendAsyncSocket() {};
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        /*
        * 获取 socket id
        */
        SOCKET GetSocketID()
        {
            return socket_id_;
        }
        /*
        * 设置 socket id
        */
        void SetSocketID(SOCKET id)
        {
            socket_id_ = id;
        }
        /*
        * 获取 persocket
        */
        virtual PerSockContext* GetPerSocket()
        {
            return nullptr;
        }
        /*
        * 将 socket 建立与 iocp 的关联,只调用一次.
        */
        virtual bool AssociateSocketToIocp()
        {
            return true;
        };
#elif defined(__linux__)
        /*
        * 获取 socket id
        */
        int32_t GetSocketID()
        {
            return socket_id_;
        }
        /*
        * 设置 socket id
        */
        void SetSocketID(int32_t id)
        {
            socket_id_ = id;
        }
        /*
        * 获取 socket 是否执行过 EPOLL_CTL_ADD
        * @return bool
        */
        bool IsCtrlAdd()
        {
            return is_ctrl_add_;
        }
        /*
        * 获取 socket 是否执行过 EPOLL_CTL_ADD
        * @params bool
        */
        void SetCtrlAdd(bool value)
        {
            is_ctrl_add_ = value;
        }
#if defined(LINUX_IO_URING)
        /*
        * 获取 uringsocket
        */
        virtual UringSockContext* GetUringSocket()
        {
            return nullptr;
        }
#endif // LINUX_IO_URING
#elif defined(__APPLE__)
        /*
        * 获取 socket id
        */
        int32_t GetSocketID()
        {
            return socket_id_;
        }
        /*
        * 设置 socket id
        */
        void SetSocketID(int32_t id)
        {
            socket_id_ = id;
        }

#endif

        /*
        * @brief 关闭
        */
        virtual void Close(ENetErrCode net_err, int32_t sys_err);
        /*
        * socket 是否有效
        */
        virtual bool IsSocketValid();
    protected:
        /*
        * 获取 socket 错误
        */
        int32_t GetSocketError();

    protected:
        uint64_t opaque_ = 0;   // 信道标记
        uint32_t conn_id_ = INVALID_CONN_ID;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        SOCKET socket_id_ = 0;
#elif defined(__linux__)
        int32_t socket_id_ = -1;                        // socket_id
        bool is_ctrl_add_ = false;                      // 是否已经执行过 EPOLL_CTL_ADD
#elif defined(__APPLE__)
        int32_t socket_id_ = -1;                        // socket_id
#endif
        int32_t event_type_ = SOCKET_EVENT_INVALID;     // socket 可响应的事件类型
        INetwork* p_network_ = nullptr;                 // 工作线程
    };

    /*
    * @brief 模拟 Nagle 算法
    * 经过压测(100字节的小包),绝大部分时候(100%),发送缓存为空,此时会直接调用系统调用send函数,造成频繁调用send函数(用户态和系统态频繁切换),反而造成了cpu居高不下,系统压力很大.
      所以这里采用 Nagle 算法思路,进行延迟发送,当积累一定数据包或者超时再进行发送操作.
    */
    struct SimulateNagle
    {
        uint32_t num_of_unsent_packets = 0; // 未发送的包的数量
        uint32_t last_send_timestamp = 0;   // 上次发送时的时间戳
        uint32_t last_recv_timestamp = 0;   // 上次接收时的时间戳
        bool flag_can_sent = true;      // 是否能够发送
        bool flag_can_recv = false;      // 是否能够接收
    };

};  // ToolBox