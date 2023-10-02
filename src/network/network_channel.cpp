#include "network_channel.h"
#include "network/network_def.h"
#include "network_def_internal.h"
#include "tools/time_util.h"
#include "event.h"
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "network/net_imp/net_iocp/tcp_iocp_network.h"
#elif defined(__linux__)
#include <sys/resource.h>
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
        event_dispatcher_ =  new EventDispatcher();
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainBinded, std::bind(&NetworkChannel::OnWorkerToMainBinded_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainAccepting, std::bind(&NetworkChannel::OnWorkerToMainAccepting_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainAccepted, std::bind(&NetworkChannel::OnWorkerToMainAccepted_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainClose, std::bind(&NetworkChannel::OnWorkerToMainClose_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainConnected, std::bind(&NetworkChannel::OnWorkerToMainConnected_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainConnectFailed, std::bind(&NetworkChannel::OnWorkerToMainConnectFailed_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainErrored, std::bind(&NetworkChannel::OnWorkerToMainErrored_, this, std::placeholders::_1));
        event_dispatcher_->RegistereventHandler(EID_WorkerToMainRecv, std::bind(&NetworkChannel::OnWorkerToMainRecv_, this, std::placeholders::_1));
    }

    NetworkChannel::~NetworkChannel()
    {
        event2main_.Clear();
        delete event_dispatcher_;
        event_dispatcher_ = nullptr;
    }

    bool NetworkChannel::Start(std::size_t net_thread_num/* = 1*/)
    {
        // 设置进程可打开的最大文件描述符数量[需要root权限才可成功]
        // SetSystemMaxOpenFiles();

        stop_.store(false);
        networks_.resize(net_thread_num);
        for (std::size_t i = 0; i < net_thread_num; i++)
        {
            NetworkLogInfo("[Network] start thread. network_thread_index:%zu", i);
            workers_.emplace_back(new std::thread([this, i]()
            {
                while (!stop_.load())
                {
                    std::time_t timetamp = GetMillSecondTimeStamp();
                    bool loaded_network = false;
                    // NetworkLogError("[Network] networks_ size :%zu", networks_.size());
                    // 驱动网络更新
                    for (auto& network : networks_[i])
                    {
                        if (network)
                        {
                            network->Update(timetamp);
                            loaded_network = true;
                        }
                    }
                    if (!loaded_network)
                    {
                        // NetworkLogWarn("[Network] Sleep. networks_ size :%zu", networks_.size());
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
                NetworkLogWarn("[Network] network has stoped. networks_ size :%zu", networks_.size());
            }));
        }
        // 将缓存的事件应用到对应的网络线程
        for (const auto& iter : cached_event_to_worker_)
        {
            NotifyWorker(std::get<2>(iter), std::get<1>(iter), std::get<0>(iter));
        }
        cached_event_to_worker_.clear();
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
        conn_type_.clear();
    }
    ENetErrCode NetworkChannel::Close(uint64_t conn_id)
    {
        auto iter = conn_type_.find(conn_id);
        if (iter == conn_type_.end())
        {
            return ENetErrCode::NET_INVALID_CONNID;
        }
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerClose);
        event->SetConnectID(conn_id);
        NotifyWorker(event, iter->second, GetNetThreadIndex(conn_id));
        conn_type_.erase(iter);
        return ENetErrCode::NET_SUCCESS;
    }
    ENetErrCode NetworkChannel::Send(uint64_t conn_id, const char* data, uint32_t size)
    {
        auto iter = conn_type_.find(conn_id);
        if (iter == conn_type_.end())
        {
            return ENetErrCode::NET_INVALID_CONNID;
        }
        auto* data_to_worker = GET_NET_MEMORY(size);
        memmove(data_to_worker, data, size);
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSend);
        event->SetConnectID(conn_id);
        event->SetData(data_to_worker, size);
        NotifyWorker(event, iter->second, GetNetThreadIndex(conn_id));
        return ENetErrCode::NET_SUCCESS;
    }

    bool NetworkChannel::NotifyMain(NetEventMain* event)
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (event2main_.Full())
        {
            NetworkLogError("[Network] Event queue is full. Drop event. network_type_:%u", event->network_type_);
            GIVE_BACK_OBJECT(event);
            return false;
        }
        event2main_.Push(std::move(event));
        return true;
    }

    void NetworkChannel::SetSimulateNagle(uint32_t packets_num /* = 10*/, uint32_t timeout/* = 2*/)
    {
        for (uint32_t net_index = 0; net_index < networks_.size(); net_index++)
        {
            auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSetSimulateNagle);
            event->SetFeatureParam(packets_num, timeout);
            NotifyWorker(event, NT_TCP, net_index);
        }
    }

    /*
    * @brief 设置绑定成功的回调
    */
    NetworkChannel& NetworkChannel::SetOnBinded(BindedMethod binded_method)
    {
        binded_ = binded_method;
        return *this;
    }
    /*
    * @brief 设置有新的连接事件的回调[尚未加入监听]
    */
    NetworkChannel& NetworkChannel::SetOnAccepting(AcceptingMethod accepting_method)
    {
        accepting_ = accepting_method;
        return *this;
    }
    /*
    * @brief 设置有新的连接事件的回调[已经加入监听]
    */
    NetworkChannel& NetworkChannel::SetOnAccepted(AcceptedMethod accepted_method)
    {
        accepted_ = accepted_method;
        return *this;
    }
    /*
    * @brief 设置主动连接成功的回调
    */
    NetworkChannel& NetworkChannel::SetOnConnected(ConnectedMethod connected_method)
    {
        connected_ = connected_method;
        return *this;
    }
    /*
    * @brief 设置主动连接失败的回调
    */
    NetworkChannel& NetworkChannel::SetOnConnectFailed(ConnectFailedMethod connect_failed_method)
    {
        connect_failed_ = connect_failed_method;
        return *this;
    }
    /*
    * @brief 设置发生错误的回调
    */
    NetworkChannel& NetworkChannel::SetOnErrored(ErroredMethod errored_method)
    {
        errored_ = errored_method;
        return *this;
    }
    /*
    * @brief 设置关闭连接的回调
    */
    NetworkChannel& NetworkChannel::SetOnClose(CloseMethod close_method)
    {
        close_ = close_method;
        return *this;
    }
    /*
    * @brief 设置接收的回调
    */
    NetworkChannel& NetworkChannel::SetOnReceived(ReceivedMethod received_method)
    {
        received_ = received_method;
        return *this;
    }

    void NetworkChannel::Accept(NetworkType type, uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewAccepter, opaque);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        NotifyWorker(event, type, 0);   // 多网络线程下,第0个线程专门作为 acceptor
    }
    void NetworkChannel::Connect(NetworkType type, uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewConnecter, opaque);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        static std::default_random_engine dre(time(0));  // 稍微随机些的种子
        std::uniform_int_distribution<unsigned > uid(0, networks_.size() - 1);
        uint32_t random_thread_index = uid(dre);
        if (0 == networks_.size())
        {
            random_thread_index = 0;
        }
        NetworkLogInfo("[Network] Push connect event to network_thread_index:%u", random_thread_index);
        NotifyWorker(event, type, random_thread_index);      //随机去某个线程中去连接.
    }
    void NetworkChannel::NotifyWorker(NetEventWorker* event, NetworkType type, uint32_t net_thread_index)
    {
        if (type >= NT_MAX || type <= NT_UNKNOWN )
        {
            OnErrored(type, 0, 0, ENetErrCode::NET_INVALID_NETWORK_TYPE, 0);
            return;
        }

        if (0 == networks_.size())
        {
            if (cached_event_to_worker_.size() > NUM_OF_CACHED_EVENT_TO_WORKER)
            {
                NetworkLogError("[Network] Vector of cache event to net thread is overflow. network_thread_index:%zu", net_thread_index);
                OnErrored(type, 0, 0, ENetErrCode::NET_CACHED_EVENT_OVERFLOW, 0);
                return;
            }
            NetworkLogInfo("[Network] Cache event for net thread. network_thread_index:%zu", net_thread_index);
            cached_event_to_worker_.emplace_back(std::make_tuple(net_thread_index, type, event));
        }

        if (net_thread_index >= networks_.size())
        {
            NetworkLogError("[Network] NotifyWorker net_thread_index:%u, networks_.size():%zu", net_thread_index, networks_.size());
            OnErrored(type, 0, 0, ENetErrCode::NET_INVALID_NET_THREAD_INDEX, 0);
            return;
        }

        auto& network_type = networks_[net_thread_index];

        auto index = static_cast<size_t>(type);
        if (nullptr == network_type[index])
        {
            network_type[index].reset(GetNetwork_(type, net_thread_index));
        }
        network_type[index]->PushEvent(std::move(event));
    }
    void NetworkChannel::OnWorkerToMainBinded_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        NetworkLogTrace("[Network] OnWorkerToMainBinded_. network_type:%u, connid:%lu", event_main->network_type_, event_main->net_evt_.accept_.connect_id_);
        OnBinded(event_main->network_type_
                 , event_main->GetOpaque()
                 , event_main->net_evt_.bind_.connect_id_
                 , event_main->GetBindIP()
                 , event_main->net_evt_.bind_.port_);
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.bind_.connect_id_] = event_main->network_type_;
        // 回调
        if (binded_)
        {
            binded_(event_main->network_type_
                    , event_main->GetOpaque()
                    , event_main->net_evt_.bind_.connect_id_
                    , event_main->GetBindIP()
                    , event_main->net_evt_.bind_.port_);
        }
    }

    void NetworkChannel::OnWorkerToMainAccepting_(Event* event)
    {
        auto* event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        int32_t fd = event_main->net_evt_.accepting_.fd_;
        NetworkType network_type = event_main->network_type_;
        OnAccepting(network_type, event_main->GetOpaque(), fd);
        if (accepting_)
        {
            accepting_(network_type, event_main->GetOpaque(), fd);
        }
        NetworkLogTrace("[Network] OnWorkerToMainAccepting_. network_type:%u, fd:%d", network_type, fd);
        JoinIOMultiplexing(network_type, event_main->GetOpaque(), fd
                           , event_main->GetAcceptingIP()
                           , event_main->net_evt_.accepting_.port_
                           , event_main->net_evt_.accepting_.send_buff_size_
                           , event_main->net_evt_.accepting_.recv_buff_size_);
    }

    void NetworkChannel::OnWorkerToMainAccepted_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        NetworkLogTrace("[Network] OnWorkerToMainAccepted_. network_type:%u, connid:%lu", event_main->network_type_, event_main->net_evt_.accept_.connect_id_);
        OnAccepted(event_main->network_type_
                   , event_main->GetOpaque()
                   , event_main->net_evt_.accept_.connect_id_);
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.accept_.connect_id_] = event_main->network_type_;
        // 回调
        if (accepted_)
        {
            accepted_(event_main->network_type_
                      , event_main->GetOpaque()
                      , event_main->net_evt_.accept_.connect_id_);
        }
    }

    void NetworkChannel::OnWorkerToMainClose_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnClose(event_main->network_type_
                , event_main->GetOpaque()
                , event_main->net_evt_.error_.connect_id_
                , event_main->net_evt_.error_.net_err_code
                , event_main->net_evt_.error_.sys_err_code);
        // 删除映射
        conn_type_.erase(event_main->net_evt_.error_.connect_id_);
        // 回调
        if (close_)
        {
            close_(event_main->network_type_
                   , event_main->GetOpaque()
                   , event_main->net_evt_.error_.connect_id_
                   , event_main->net_evt_.error_.net_err_code
                   , event_main->net_evt_.error_.sys_err_code);
        }
    }

    void NetworkChannel::OnWorkerToMainConnected_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnConnected(event_main->network_type_
                    , event_main->GetOpaque()
                    , event_main->net_evt_.connect_sucessed_.connect_id_);
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.connect_sucessed_.connect_id_] = event_main->network_type_;
        // 回调
        if (connected_)
        {
            connected_(event_main->network_type_
                       , event_main->GetOpaque()
                       , event_main->net_evt_.connect_sucessed_.connect_id_);
        }
    }

    void NetworkChannel::OnWorkerToMainConnectFailed_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnConnectedFailed(event_main->network_type_
                          , event_main->GetOpaque()
                          , event_main->net_evt_.connect_failed_.net_err_code
                          , event_main->net_evt_.connect_failed_.sys_err_code);
        if (connect_failed_)
        {
            connect_failed_(event_main->network_type_
                            , event_main->GetOpaque()
                            , event_main->net_evt_.connect_failed_.net_err_code
                            , event_main->net_evt_.connect_failed_.sys_err_code);
        }

    }

    void NetworkChannel::OnWorkerToMainErrored_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnErrored(event_main->network_type_
                  , event_main->GetOpaque()
                  , event_main->net_evt_.error_.connect_id_
                  , event_main->net_evt_.error_.net_err_code
                  , event_main->net_evt_.error_.sys_err_code);
        if (errored_)
        {
            errored_(event_main->network_type_
                     , event_main->GetOpaque()
                     , event_main->net_evt_.error_.connect_id_
                     , event_main->net_evt_.error_.net_err_code
                     , event_main->net_evt_.error_.sys_err_code);
        }
    }

    void NetworkChannel::OnWorkerToMainRecv_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        OnReceived(event_main->network_type_
                   , event_main->GetOpaque()
                   , event_main->net_evt_.recv_.connect_id_
                   , event_main->net_evt_.recv_.data_
                   , event_main->net_evt_.recv_.size_);
        if (received_)
        {
            received_(event_main->network_type_
                      , event_main->GetOpaque()
                      , event_main->net_evt_.recv_.connect_id_
                      , event_main->net_evt_.recv_.data_
                      , event_main->net_evt_.recv_.size_);
        }
    }



    void NetworkChannel::DispatchMainEvent_()
    {
        while (!event2main_.Empty())
        {
            NetEventMain* event = event2main_.Pop();
            if (nullptr == event)
            {
                OnErrored(NT_UNKNOWN, 0, 0, ENetErrCode::NET_INVALID_EVENT, 0);
                continue;
            }
            event_dispatcher_ ->HandleEvent(event);
            GIVE_BACK_OBJECT(event);
        }
    }

    INetwork* NetworkChannel::GetNetwork_(NetworkType type, uint32_t net_thread_index)
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
                tcp_network->Init(this, type, net_thread_index);
                return tcp_network;
                break;
            }
            case NT_UDP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                udp_network->Init(this, type, net_thread_index);
                return udp_network;
#endif // __linux__
                break;
            }
            case NT_KCP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                udp_network->Init(this, type, net_thread_index);
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

    void NetworkChannel::JoinIOMultiplexing(NetworkType type, uint64_t opaque, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerJoinIOMultiplexing, opaque);
        event->SetFd(fd);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        static std::default_random_engine dre(time(0));  // 稍微随机些的种子
        std::uniform_int_distribution<unsigned > uid(0, networks_.size() - 1);
        uint32_t random_thread_index = uid(dre);
        NotifyWorker(event, type, random_thread_index);      //随机去某个线程中去连接.
    }

    void NetworkChannel::SetSystemMaxOpenFiles()
    {
#if defined(__linux__)
        struct rlimit limit;
        limit.rlim_cur = 40960;
        limit.rlim_max = 65535;
        int32_t ret = setrlimit(RLIMIT_NOFILE, &limit);
        if (-1 == ret)
        {
            NetworkLogInfo("[Network] SetSystemMaxOpenFiles {40960,65535} FAILED! NEED ROOT! fd:%d", ret);
        }
        else
        {
            NetworkLogInfo("[Network] SetSystemMaxOpenFiles {40960,65535} success.result fd:%d", ret);
        }

#endif // __linux__
    }


    /////////////////////////////// network/network_api.h 实现部分 ///////////////////////////////
    Network::Network()
    {
        if (nullptr == network_channel_)
        {
            network_channel_ = new NetworkChannel();
        }
    }

    Network::~Network()
    {
        if (nullptr != network_channel_)
        {
            delete network_channel_;
            network_channel_ = nullptr;
        }
    }

    bool Network::Start(std::size_t net_thread_num /* = 1 */)
    {
        return network_channel_->Start(net_thread_num);
    }

    void Network::Update()
    {
        network_channel_->Update();
    }

    void Network::StopWait()
    {
        network_channel_->StopWait();
    }

    ENetErrCode Network::Close(uint64_t conn_id)
    {
        return network_channel_->Close(conn_id);
    }

    ENetErrCode Network::Send(uint64_t conn_id, const char* data, uint32_t size)
    {
        return network_channel_->Send(conn_id, data, size);
    }

    void Network::Accept(NetworkType type, uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size /*= 0*/, int32_t recv_buff_size /*= 0*/)
    {
        network_channel_->Accept(type, opaque, ip, port, send_buff_size, recv_buff_size);
    }

    void Network::Connect(NetworkType type, uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size /*= 0*/, int32_t recv_buff_size /*= 0*/)
    {
        network_channel_->Connect(type, opaque, ip, port, send_buff_size, recv_buff_size);
    }

    void Network::SetSimulateNagle(uint32_t packets_num /*= 10*/, uint32_t timeout /*= 2*/)
    {
        network_channel_->SetSimulateNagle(packets_num, timeout);
    }

    Network& Network::SetOnBinded(BindedMethod binded_method)
    {
        network_channel_->SetOnBinded(binded_method);
        return *this;
    }

    Network& Network::SetOnAccepting(AcceptingMethod accepting_method)
    {
        network_channel_->SetOnAccepting(accepting_method);
        return *this;
    }

    Network& Network::SetOnAccepted(AcceptedMethod accepted_method)
    {
        network_channel_->SetOnAccepted(accepted_method);
        return *this;
    }

    Network& Network::SetOnConnected(ConnectedMethod connected_method)
    {
        network_channel_->SetOnConnected(connected_method);
        return *this;
    }

    Network& Network::SetOnConnectFailed(ConnectFailedMethod connect_failed_method)
    {
        network_channel_->SetOnConnectFailed(connect_failed_method);
        return *this;
    };

    Network& Network::SetOnErrored(ErroredMethod errored_method)
    {
        network_channel_->SetOnErrored(errored_method);
        return *this;
    };

    Network& Network::SetOnClose(CloseMethod close_method)
    {
        network_channel_->SetOnClose(close_method);
        return *this;
    };

    Network& Network::SetOnReceived(ReceivedMethod receive_method)
    {
        network_channel_->SetOnReceived(receive_method);
        return *this;
    };

};  // ToolBox