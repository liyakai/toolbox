#include "network_channel.h"
#include <cstddef>
#include <cstdint>
#include <random>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "network/net_imp/net_iocp/tcp_iocp_network.h"
#elif defined(__linux__)
#if defined(LINUX_IO_URING)
#include "network/net_imp/net_io_uring/tcp_io_uring_network.h"
#else
#endif // LINUX_IO_URING
#include "network/net_imp/net_epoll/tcp_epoll_network.h"
#include "network/net_imp/net_epoll/udp_epoll_network.h"
#elif defined(__APPLE__)
#include "network/net_imp/net_kqueue/tcp_kqueue_network.h"
#endif

namespace ToolBox
{

    NetworkChannel::NetworkChannel()
    {
        stop_.store(false);
        RegistereventHandler(EID_WorkerToMainAcceptting, std::bind(&NetworkChannel::OnWorkerToMainAcceptting_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainAccepted, std::bind(&NetworkChannel::OnWorkerToMainAccepted_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainClose, std::bind(&NetworkChannel::OnWorkerToMainClose_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainConnected, std::bind(&NetworkChannel::OnWorkerToMainConnected_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainConnectFailed, std::bind(&NetworkChannel::OnWorkerToMainConnectFailed_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainErrored, std::bind(&NetworkChannel::OnWorkerToMainErrored_, this, std::placeholders::_1));
        RegistereventHandler(EID_WorkerToMainRecv, std::bind(&NetworkChannel::OnWorkerToMainRecv_, this, std::placeholders::_1));
    }

    NetworkChannel::~NetworkChannel()
    {
        event2main_.Clear();
    }

    bool NetworkChannel::Start(std::size_t net_thread_num/* = 1*/)
    {
        stop_.store(false);
        for (std::size_t i = 0; i < net_thread_num; i++)
        {
            NetworkLogInfo("[Network] start thread. network_thread_index:%d", i);
            workers_.emplace_back(new std::thread([this]()
            {
                while (!stop_.load())
                {
                    // NetworkLogError("[Network] networks_ size :%zu", networks_.size());
                    // 驱动网络更新
                    for (auto& networks : networks_)
                    {
                        for (auto& network : networks)
                        {
                            if (network)
                            {
                                // NetworkLogError("[Network] Update. network_type:%d", network->GetNetworkType());
                                network->Update();
                            }
                        }
                    }
                }
                NetworkLogWarn("[Network] network has stoped. networks_ size :%zu", networks_.size());
            }));
        }


        return true;
    }
    void NetworkChannel::Update()
    {
        DispatchMainEvent_();
    }

    void NetworkChannel::StopWait()
    {
        stop_.store(true);
        for (auto& workder : workers_)
        {
            if (workder)
            {
                workder->join();
                workder.reset();
            }
        }

        while (!event2main_.Empty())
        {
            GIVE_BACK_OBJECT(event2main_.Pop());
        }
        for (auto& networks : networks_)
        {
            for (auto& network : networks)
            {
                if (nullptr != network)
                {
                    network->UnInit();
                }
                network.reset();
            }
        }

    }
    void NetworkChannel::Close(NetworkType type, uint64_t conn_id)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerClose);
        event->SetConnectID(conn_id);
        NotifyWorker(event, type, GetNetThreadIndex(conn_id));
    }
    void NetworkChannel::Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size)
    {
        auto* data_to_worker = GET_NET_MEMORY(size);
        memmove(data_to_worker, data, size);
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSend);
        event->SetConnectID(conn_id);
        event->SetData(data_to_worker, size);
        NotifyWorker(event, type, GetNetThreadIndex(conn_id));
    }

    bool NetworkChannel::NotifyMain(NetEventMain* event)
    {
        if (!event2main_.Full())
        {
            event2main_.Push(std::move(event));
            return true;
        }
        return false;
    }

    void NetworkChannel::Accept(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewAccepter);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        NotifyWorker(event, type, 0);   // 多网络线程下,第0个线程专门作为 acceptor
    }
    void NetworkChannel::Connect(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewConnecter);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        static std::default_random_engine dre(time(0));  // 稍微随机些的种子
        std::uniform_int_distribution<unsigned > uid(0, networks_.size() - 1);
        uint32_t random_thread_index = uid(dre);
        NetworkLogInfo("[Network] Push connect event to network_thread_index:%u", random_thread_index);
        NotifyWorker(event, type, random_thread_index);      //随机去某个线程中去连接.
    }
    void NetworkChannel::NotifyWorker(NetEventWorker* event, NetworkType type, uint32_t net_thread_index)
    {
        if (type >= NT_MAX || type <= NT_UNKNOWN )
        {
            OnErrored(type, 0, ENetErrCode::NET_INVALID_NETWORK_TYPE, 0);
            return;
        }

        if (net_thread_index >= networks_.size())
        {
            OnErrored(type, 0, ENetErrCode::NET_INVALID_NET_THREAD_INDEX, 0);
            return;
        }

        auto& network_type = networks_[net_thread_index];

        auto index = static_cast<size_t>(type);
        if (nullptr == network_type[index])
        {
            network_type[index].reset(GetNetwork_(type));
        }
        network_type[index]->PushEvent(std::move(event));
    }


    void NetworkChannel::OnWorkerToMainAcceptting_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        int32_t fd = event_main->net_evt_.acceptting_.fd_;
        NetworkType network_type = event_main->network_type_;
        OnAcceptting(network_type, fd);
        JoinIOMultiplexing(network_type, fd
                           , event_main->GetIP()
                           , event_main->net_evt_.acceptting_.port_
                           , event_main->net_evt_.acceptting_.send_buff_size_
                           , event_main->net_evt_.acceptting_.recv_buff_size_);
    }

    void NetworkChannel::OnWorkerToMainAccepted_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnAccepted(event_main->network_type_
                   , event_main->net_evt_.accept_.connect_id_);
    }

    void NetworkChannel::OnWorkerToMainClose_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnClose(event_main->network_type_,
                event_main->net_evt_.error_.connect_id_,
                event_main->net_evt_.error_.net_err_code,
                event_main->net_evt_.error_.sys_err_code);
    }

    void NetworkChannel::OnWorkerToMainConnected_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnConnected(event_main->network_type_
                    , event_main->net_evt_.connect_sucessed_.connect_id_);
    }

    void NetworkChannel::OnWorkerToMainConnectFailed_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnConnectedFailed(event_main->network_type_,
                          event_main->net_evt_.connect_failed_.net_err_code,
                          event_main->net_evt_.connect_failed_.sys_err_code);
    }

    void NetworkChannel::OnWorkerToMainErrored_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnErrored(event_main->network_type_,
                  event_main->net_evt_.error_.connect_id_,
                  event_main->net_evt_.error_.net_err_code,
                  event_main->net_evt_.error_.sys_err_code);
    }

    void NetworkChannel::OnWorkerToMainRecv_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnReceived(event_main->network_type_,
                   event_main->net_evt_.recv_.connect_id_,
                   event_main->net_evt_.recv_.data_,
                   event_main->net_evt_.recv_.size_);
    }



    void NetworkChannel::DispatchMainEvent_()
    {
        while (!event2main_.Empty())
        {
            NetEventMain* event = event2main_.Pop();
            if (nullptr == event)
            {
                OnErrored(NT_UNKNOWN, 0, ENetErrCode::NET_INVALID_EVENT, 0);
                continue;
            }
            HandleEvent(event);
            GIVE_BACK_OBJECT(event);
        }
    }

    INetwork* NetworkChannel::GetNetwork_(NetworkType type)
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

    uint32_t NetworkChannel::GetNetThreadIndex(uint64_t conn_id)
    {
        //这里是解码,编码方式见 socket_pool.h  MakeUint32
        return (conn_id >> 16) & 0x00FF;
    }

    void NetworkChannel::JoinIOMultiplexing(NetworkType type, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerJoinIOMultiplexing);
        event->SetFd(fd);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        static std::default_random_engine dre(time(0));  // 稍微随机些的种子
        std::uniform_int_distribution<unsigned > uid(0, networks_.size() - 1);
        uint32_t random_thread_index = uid(dre);
        NetworkLogInfo("[Network] Push JoinIOMultiplexing event to network_thread_index:%u", random_thread_index);
        NotifyWorker(event, type, random_thread_index);      //随机去某个线程中去连接.
    }

};  // ToolBox