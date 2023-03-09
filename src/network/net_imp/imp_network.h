#pragma once
#include "network/network.h"
#include "network/network_def.h"
#include "socket_pool.h"
#include "base_ctrl.h"
#include "tcp_socket.h"
#include "tools/virtual_print.h"
#include "udp_socket.h"
#include <cstdint>

namespace ToolBox
{

    /*
    * @file brief 网络基类 INetwork 作为纯虚类,更多的是作为接口的存在,不关心实现细节.但各种类型的具体实现又有很多相同的地方.
    * 故,中间增加一层实现层,将据悉实现共通的部分提出,单独作为一层.
    * 2. 也要注意到,在OnNewAccepter和OnNewConnecter函数中,子类使用socket时增加了一层间接性,即需要根据ID取用socket对象,不过这部分
    * 代码不是热点代码,这层效率为O(1)的间接性对网络库性能的影响可以忽略不计.
    */


    /*
    * 定义实现层 network
    */
    template<typename SocketType>
    class ImpNetwork : public INetwork
    {
    public:
        /*
        * @brief 构造
        */
        ImpNetwork();
        /*
        * @brief 析构
        */
        virtual ~ImpNetwork();
        /*
        * @brief 初始化函数
        * @param NetworkChannel* 主线程
        * @param NetworkType 网络类型
        */
        virtual bool Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index) override;
        /*
        * @brief 逆初始化网络
        */
        virtual bool UnInit() override;
        /*
        * @brief 执行一次网络循环
        */
        virtual void Update(std::time_t time_stamp) override;
        /*
        * @brief 获取控制器
        */
        IOMultiplexingInterface* GetBaseCtrl()
        {
            return base_ctrl_;
        }
        /*
        * @brief 从IO多路复用种删去监听
        */
        virtual void CloseListenInMultiplexing(int32_t socket_id) override;

    protected:
        /*
        * 工作线程内建立监听器
        */
        virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
        /*
        * 主线程通知,将fd加入io多路复用
        */
        virtual uint64_t OnJoinIOMultiplexing(int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
        /*
        * 工作线程内建立连接器
        */
        virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
        /*
        * 工作线程内闭网络连接
        */
        virtual void OnClose(uint64_t connect_id) override;
        /*
        * 工作线程内工作线程内发送
        */
        virtual void OnSend(uint64_t connect_id, const char* data, std::size_t size) override;
    protected:
        SocketPool<SocketType> sock_mgr_;       // socket 池
        IOMultiplexingInterface* base_ctrl_;    // io多路复用接口
        std::time_t last_update_timestamp = 0;  // 上次update时的时间戳
    };

    template<typename SocketType>
    ImpNetwork<SocketType>::ImpNetwork()
    {

    }

    template<typename SocketType>
    ImpNetwork<SocketType>::~ImpNetwork()
    {

    }

    template<typename SocketType>
    bool ImpNetwork<SocketType>::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        if (!INetwork::Init(master, network_type, net_thread_index))
        {
            NetworkLogError("[Network] Init INetwork failed. network_type:%d", network_type);
            return false;
        }
        if (!sock_mgr_.Init(MAX_SOCKET_COUNT, net_thread_index))
        {
            NetworkLogError("[Network] Init sock_mgr_ failed. network_type:%d", network_type);
            return false;
        }
        if (!base_ctrl_->CreateIOMultiplexing())
        {
            NetworkLogError("[Network] CreateIOMultiplexing failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

    template<typename SocketType>
    bool ImpNetwork<SocketType>::UnInit()
    {
        if (nullptr != base_ctrl_)
        {
            base_ctrl_->DestroyIOMultiplexing();
            base_ctrl_ = nullptr;
        }
        if (!sock_mgr_.UnInit())
        {
            NetworkLogError("[Network] sock_mgr_ UnInit failed. network_type:%d", GetNetworkType());
            return false;
        }
        if (!INetwork::UnInit())
        {
            NetworkLogError("[Network] INetwork UnInit failed. network_type:%d", GetNetworkType());
            return false;
        }
        return true;
    }

    template<typename SocketType>
    void ImpNetwork<SocketType>::Update(std::time_t time_stamp)
    {

        // NetworkLogInfo("[Network] Update. network_type:%d,time_stamp:%lld, last_update_timestamp:%lld", GetNetworkType(), time_stamp, last_update_timestamp);
        INetwork::Update(time_stamp);
        base_ctrl_->RunOnce(time_stamp);

        int32_t nagle_timeout = GetSimulateNagleTimeout();
        // NetworkLogDebug("[Network] Update. network_type:%d, nagle_timeout:%d, time_stamp:%lld, last_update_timestamp:%lld", GetNetworkType(), nagle_timeout, time_stamp, last_update_timestamp);
        if (nagle_timeout > 0 && time_stamp >= last_update_timestamp + nagle_timeout)
        {
            // NetworkLogDebug("[Network] Update. network_type:%d, nagle_timeout:%d, time_stamp:%lld, last_update_timestamp:%lld", GetNetworkType(), nagle_timeout, time_stamp, last_update_timestamp);
            last_update_timestamp = time_stamp;
            sock_mgr_.Foreach([time_stamp](SocketType * socket) -> bool
            {
                if (socket)
                {
                    socket->Update(time_stamp);
                }
                return true;
            });
        }


    }

    template<typename SocketType>
    void ImpNetwork<SocketType>::CloseListenInMultiplexing(int32_t socket_id)
    {
        base_ctrl_->DelEvent(socket_id);
    }

    template<typename SocketType>
    uint64_t ImpNetwork<SocketType>::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        SocketType* new_socket = sock_mgr_.Alloc();
        if (nullptr == new_socket)
        {
            OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
            return INVALID_CONN_ID;
        }
        new_socket->SetSocketMgr(&sock_mgr_);
        new_socket->SetNetwork(this);
        if (false == new_socket->InitNewAccepter(ip, port, send_buff_size, recv_buff_size))
        {
            OnErrored(0, ENetErrCode::NET_ACCEPT_FAILED, 0);
            return INVALID_CONN_ID;
        }
        base_ctrl_->OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    }
    template<typename SocketType>
    uint64_t ImpNetwork<SocketType>::OnJoinIOMultiplexing(int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto new_socket = sock_mgr_.Alloc();
        if (nullptr == new_socket)
        {
            OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
            return INVALID_CONN_ID;
        }
        new_socket->SetSocketMgr(&sock_mgr_);
        new_socket->SetNetwork(this);

        if (false == new_socket->InitAccpetSocket(fd, ip, port, send_buff_size, recv_buff_size))
        {
            sock_mgr_.Free(new_socket);
            return INVALID_CONN_ID;
        }
        OnAccepted(new_socket->GetConnID());
        base_ctrl_->OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        NetworkLogTrace("[Network] OnJoinIOMultiplexing. fd:%d, connid:%u", fd, new_socket->GetConnID());
        return new_socket->GetConnID();
    }
    template<typename SocketType>
    uint64_t ImpNetwork<SocketType>::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto new_socket = sock_mgr_.Alloc();
        if (nullptr == new_socket)
        {
            OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
            return INVALID_CONN_ID;
        }
        new_socket->SetSocketMgr(&sock_mgr_);
        new_socket->SetNetwork(this);
        if (false == new_socket->InitNewConnecter(ip, port, send_buff_size, recv_buff_size))
        {
            return INVALID_CONN_ID;
        }
        base_ctrl_->OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    }
    template<typename SocketType>
    void ImpNetwork<SocketType>::OnClose(uint64_t connect_id)
    {
        auto socket = sock_mgr_.GetSocket((uint32_t)connect_id);
        if (nullptr == socket)
        {
            return;
        }
        socket->Close(ENetErrCode::NET_NO_ERROR);
    }
    template<typename SocketType>
    void ImpNetwork<SocketType>::OnSend(uint64_t connect_id, const char* data, std::size_t size)
    {
        auto socket = sock_mgr_.GetSocket(connect_id);
        if (nullptr == socket)
        {
            return;
        }
        socket->Send(data, size);
    }

};  // ToolBox