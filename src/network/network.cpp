#include "network.h"
#include "event.h"
#include "network_def.h"
#include <functional>

namespace ToolBox
{

    INetwork::INetwork()
    {
        RegistereventHandler(EID_MainToWorkerNewAccepter, std::bind(&INetwork::OnMainToWorkerNewAccepter_, this, std::placeholders::_1));
        RegistereventHandler(EID_MainToWorkerJoinIOMultiplexing, std::bind(&INetwork::OnMainToWorkerJoinIOMultiplexing_, this, std::placeholders::_1));
        RegistereventHandler(EID_MainToWorkerNewConnecter, std::bind(&INetwork::OnMainToWorkerNewConnecter_, this, std::placeholders::_1));
        RegistereventHandler(EID_MainToWorkerSend, std::bind(&INetwork::OnMainToWorkerSend_, this, std::placeholders::_1));
        RegistereventHandler(EID_MainToWorkerClose, std::bind(&INetwork::OnMainToWorkerClose_, this, std::placeholders::_1));
        RegistereventHandler(EID_MainToWorkerSetSimulateNagle, std::bind(&INetwork::SetSimulateNagle_, this, std::placeholders::_1));

    }

    INetwork::~INetwork()
    {
        while (!event2worker_.Empty())
        {
            NetEventWorker* event = event2worker_.Pop();
            if (!event)
            {
                GIVE_BACK_OBJECT(event);
            }

        }
        event2worker_.Clear();
    }

    bool INetwork::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        master_ = master;
        network_type_ = network_type;
        net_thread_index_ = net_thread_index;
        return true;
    }
    bool INetwork::UnInit()
    {
        return true;
    }
    void INetwork::Update(std::time_t time_stamp)
    {
        update_timestamp_ = time_stamp;
        // 处理主线程发来的事件
        HandleEvents_();
    }

    void INetwork::PushEvent(NetEventWorker* event)
    {
        if (event2worker_.Full())
        {
            NetworkLogError("[Network] Event queue is full. Drop event. network_type_:%u", event->GetNetworkType());
            GIVE_BACK_OBJECT(event);
            return;
        }
        event2worker_.Push(std::move(event));
    }

    void INetwork::OnAcceptting(int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* accept_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainAcceptting);
        accept_event->network_type_ = network_type_;
        accept_event->net_evt_.acceptting_.fd_ = fd;
        accept_event->SetAccepttingIP(ip);
        accept_event->net_evt_.acceptting_.port_ = port;
        accept_event->net_evt_.acceptting_.send_buff_size_ = send_buff_size;
        accept_event->net_evt_.acceptting_.recv_buff_size_ = recv_buff_size;
        master_->NotifyMain(accept_event);
    }
    void INetwork::OnAccepted(uint64_t connect_id)
    {
        auto* accept_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainAccepted);
        accept_event->network_type_ = network_type_;
        accept_event->net_evt_.accept_.connect_id_ = connect_id;
        master_->NotifyMain(accept_event);
    }
    void INetwork::OnConnected(uint64_t connect_id)
    {
        auto* connected_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainConnected);
        connected_event->network_type_ = network_type_;
        connected_event->net_evt_.connect_sucessed_.connect_id_ = connect_id;
        master_->NotifyMain(connected_event);
    }

    void INetwork::OnConnectedFailed(ENetErrCode err_code, int32_t err_no)
    {
        auto* connected_failed_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainConnectFailed);
        connected_failed_event->network_type_ = network_type_;
        connected_failed_event->net_evt_.connect_failed_.net_err_code = err_code;
        connected_failed_event->net_evt_.connect_failed_.sys_err_code = err_no;
        master_->NotifyMain(connected_failed_event);
    }

    void INetwork::OnErrored(uint64_t connect_id, ENetErrCode err_code, int32_t err_no)
    {
        auto* errored_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainErrored);
        errored_event->network_type_ = network_type_;
        errored_event->net_evt_.error_.connect_id_ = connect_id;
        errored_event->net_evt_.error_.net_err_code = err_code;
        errored_event->net_evt_.error_.sys_err_code = err_no;
        master_->NotifyMain(errored_event);
    }

    void INetwork::OnClosed(uint64_t connect_id, ENetErrCode err_code, int32_t err_no)
    {
        auto* close_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainClose);
        close_event->network_type_ = network_type_;
        close_event->net_evt_.close_.connect_id_ = connect_id;
        close_event->net_evt_.close_.net_err_ = err_code;
        close_event->net_evt_.close_.sys_err_ = err_no;
        master_->NotifyMain(close_event);
    }

    void INetwork::OnReceived(uint64_t connect_id, const char* data, uint32_t size)
    {
        auto* receive_event = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainRecv);
        receive_event->network_type_ = network_type_;
        receive_event->net_evt_.recv_.connect_id_ = connect_id;
        receive_event->net_evt_.recv_.data_ = data;
        receive_event->net_evt_.recv_.size_ = size;
        master_->NotifyMain(receive_event);
    }

    int32_t INetwork::GetSimulateNaglePacketsNum()
    {
        return nagle_packets_num_;
    }
    int32_t INetwork::GetSimulateNagleTimeout()
    {
        return nagle_timeout_;
    }

    void INetwork::OnMainToWorkerNewAccepter_(Event* event)
    {
        auto accepter_event = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == accepter_event)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        auto conn_id = OnNewAccepter(accepter_event->GetIP(), accepter_event->GetPort(), accepter_event->GetSendBuffSize(), accepter_event->GetRecvBuffSize());
        auto bind_tcp = GET_NET_OBJECT(NetEventMain, EID_WorkerToMainBinded);
        bind_tcp->network_type_ = network_type_;
        bind_tcp->net_evt_.bind_.connect_id_ = conn_id;
        bind_tcp->SetBindIP(accepter_event->GetIP());
        bind_tcp->net_evt_.bind_.port_ = accepter_event->GetPort();
        master_->NotifyMain(bind_tcp);
    }

    void INetwork::OnMainToWorkerJoinIOMultiplexing_(Event* event)
    {
        auto acceptting_event = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == acceptting_event)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        NetworkLogTrace("[Network] OnMainToWorkerJoinIOMultiplexing_  fd:%d", acceptting_event->GetFd());
        OnJoinIOMultiplexing(acceptting_event->GetFd(), acceptting_event->GetIP(), acceptting_event->GetPort(), acceptting_event->GetSendBuffSize(), acceptting_event->GetRecvBuffSize());
    }

    void INetwork::OnMainToWorkerNewConnecter_(Event* event)
    {
        auto connecter_tcp = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == connecter_tcp)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        OnNewConnecter(connecter_tcp->GetIP(), connecter_tcp->GetPort(), connecter_tcp->GetSendBuffSize(), connecter_tcp->GetRecvBuffSize());
    }

    void INetwork::OnMainToWorkerClose_(Event* event)
    {
        auto close_tcp = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == close_tcp)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        OnClose(close_tcp->GetConnectID());
    }

    void INetwork::OnMainToWorkerSend_(Event* event)
    {
        auto send = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == send)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        OnSend(send->GetConnectID(), send->GetData(), send->GetDataSize());
    }

    void INetwork::SetSimulateNagle_(Event* event)
    {
        auto set_nagle_event = dynamic_cast<NetEventWorker*>(event);
        if (nullptr == set_nagle_event)
        {
            NetworkLogError("[Network] event is null.");
            return;
        }
        const auto& nagle_params = set_nagle_event->GetFeatureParam();
        nagle_packets_num_ = std::get<0>(nagle_params);
        nagle_timeout_ = std::get<1>(nagle_params);
        NetworkLogDebug("[Network] Set nagle_packets_num_:%d, set nagle_timeout_:%d.", nagle_packets_num_, nagle_timeout_);
    }

    void INetwork::HandleEvents_()
    {
        while (!event2worker_.Empty())
        {
            NetEventWorker* event = event2worker_.Pop();
            if (nullptr != event)
            {
                HandleEvent(event);
                GIVE_BACK_OBJECT(event);
            }
            else
            {
                OnErrored(0, ENetErrCode::NET_INVALID_EVENT, 0);
            }
        }
    }

    uint32_t INetwork::GetThreadIndex()
    {
        return net_thread_index_;
    }

};  // ToolBox