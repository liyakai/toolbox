#include "network_mgr.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "src/network/net_imp/net_iocp/tcp_iocp_network.h"
#elif defined(__linux__)
#if defined(LINUX_IO_URING)
#include "src/network/net_imp/net_io_uring/tcp_io_uring_network.h"
#else
#endif // LINUX_IO_URING
#include "src/network/net_imp/net_epoll/tcp_epoll_network.h"
#include "src/network/net_imp/net_epoll/udp_epoll_network.h"
#elif defined(__APPLE__)
#include "src/network/net_imp/net_kqueue/tcp_kqueue_network.h"
#endif

namespace ToolBox
{

    NetworkMaster::NetworkMaster()
    {
        stop_.store(false);
    }

    NetworkMaster::~NetworkMaster()
    {
        event2main_.Clear();
    }

    bool NetworkMaster::Start()
    {
        stop_.store(false);
        worker_.reset(new std::thread([this]()
        {
            while (!stop_.load())
            {
                // 驱动网络更新
                for (auto& network : networks_)
                {
                    if (network)
                    {
                        network->Update();
                    }
                }
            }
        }));
        return true;
    }
    void NetworkMaster::Update()
    {
        DispatchMainEvent_();
    }

    void NetworkMaster::StopWait()
    {
        stop_.store(true);
        if (worker_)
        {
            worker_->join();
            worker_.reset();
        }
        while (!event2main_.Empty())
        {
            GIVE_BACK_OBJECT(event2main_.Pop());
        }
        for (auto& network : networks_)
        {
            if (nullptr != network)
            {
                network->UnInit();
            }
            network.reset();
        }

    }
    void NetworkMaster::Close(NetworkType type, uint64_t conn_id)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerClose);
        event->SetConnectID(conn_id);
        NotifyWorker(event, type);
    }
    void NetworkMaster::Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size)
    {
        auto* data_to_worker = GET_NET_MEMORY(size);
        memmove(data_to_worker, data, size);
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSend);
        event->SetConnectID(conn_id);
        event->SetData(data_to_worker, size);
        NotifyWorker(event, type);
    }

    void NetworkMaster::Accept(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewAccepter);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        NotifyWorker(event, type);
    }
    void NetworkMaster::Connect(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewConnecter);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        NotifyWorker(event, type);
    }
    void NetworkMaster::NotifyWorker(NetEventWorker* event, NetworkType type)
    {
        if (type >= NT_MAX || type <= NT_UNKNOWN )
        {
            OnErrored(0, ENetErrCode::NET_INVALID_NETWORK_TYPE, 0);
            return;
        }

        auto index = static_cast<size_t>(type);
        if (nullptr == networks_[index])
        {
            networks_[index].reset(GetNetwork_(type));
        }
        networks_[index]->PushEvent(std::move(event));
    }
    bool NetworkMaster::NotifyMain(NetEventMain* event)
    {
        if (!event2main_.Full())
        {
            event2main_.Push(std::move(event));
            return true;
        }
        return false;
    }



    void NetworkMaster::DispatchMainEvent_()
    {
        while (!event2main_.Empty())
        {
            NetEventMain* event = event2main_.Pop();
            if (nullptr == event)
            {
                OnErrored(0, ENetErrCode::NET_INVALID_SOCKET, 0);
                continue;
            }
            switch (event->GetID())
            {
                case EID_WorkerToMainBinded:
                    break;
                case EID_WorkerToMainAccepted:
                    OnAccepted(event->net_evt_.accept_.connect_id_);
                    break;
                case EID_WorkerToMainClose:
                    OnClose(event->net_evt_.error_.connect_id_,
                            event->net_evt_.error_.net_err_code,
                            event->net_evt_.error_.sys_err_code);
                    break;
                case EID_WorkerToMainConnected:
                    OnConnected(event->net_evt_.connect_sucessed_.connect_id_);
                    break;
                case EID_WorkerToMainConnectFailed:
                    OnConnectedFailed(event->net_evt_.connect_failed_.net_err_code,
                                      event->net_evt_.connect_failed_.sys_err_code);
                    break;
                case EID_WorkerToMainErrored:
                    OnErrored(event->net_evt_.error_.connect_id_,
                              event->net_evt_.error_.net_err_code,
                              event->net_evt_.error_.sys_err_code);
                    break;
                case EID_WorkerToMainRecv:
                    OnReceived(event->net_evt_.recv_.connect_id_,
                               event->net_evt_.recv_.data_,
                               event->net_evt_.recv_.size_);
                    break;
                default:
                    break;
            }
            GIVE_BACK_OBJECT(event);
        }
    }

    INetwork* NetworkMaster::GetNetwork_(NetworkType type)
    {
        switch (type)
        {
            case NT_TCP:
            {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                auto* tcp_network = new TcpIocpNetwork();
#elif defined(__linux__)
#if defined(LINUX_IO_URING)
                auto* tcp_network = new TcpIOUringNetwork();
#else
                auto* tcp_network = new TcpEpollNetwork();
#endif // LINUX_IO_URING
#elif defined(__APPLE__)
                auto* tcp_network = new TcpKqueueNetwork();
#endif
                tcp_network->Init(this, type);
                return tcp_network;
                break;
            }
            case NT_UDP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                udp_network->Init(this, type);
                return udp_network;
#endif // __linux__
                break;
            }
            case NT_KCP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                udp_network->Init(this, type);
                udp_network->OpenKcpMode();
                return udp_network;
#endif // __linux__
                break;
            }
            case NT_UNKNOWN:
            case NT_MAX:
            default:
                break;
        }
        return nullptr;
    }

};  // ToolBox