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
#include <unistd.h>
#if defined(LINUX_IO_URING)
#include "network/net_imp/net_io_uring/tcp_io_uring_network.h"
#else
#endif // LINUX_IO_URING
#include "network/net_imp/net_epoll/tcp_epoll_network.h"
#include "network/net_imp/net_epoll/udp_epoll_network.h"
#elif defined(__APPLE__)
#include <unistd.h>
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
        // 兜底停止线程,避免调用方遗漏 StopWait 导致 joinable thread 析构
        StopWait();
        delete event_dispatcher_;
        event_dispatcher_ = nullptr;
    }

    bool NetworkChannel::IsNetworkTypeSupported_(NetworkType type)
    {
        switch (type)
        {
            case NT_TCP:
                return true;
            case NT_UDP:
            case NT_KCP:
#if defined(__linux__)
                return true;
#else
                return false;
#endif
            default:
                return false;
        }
    }

    void NetworkChannel::RollbackStart_()
    {
        stop_.store(true);
        for (auto& worker : workers_)
        {
            if (worker)
            {
                worker->join();
                worker.reset();
            }
        }
        workers_.clear();
        for (auto& networks : networks_)
        {
            for (auto& network : networks)
            {
                if (network)
                {
                    network->UnInit();
                }
                network.reset();
            }
        }
        networks_.clear();
        workers_ready_.store(false);
        stop_.store(false);
    }

    void NetworkChannel::DrainMainEvents_()
    {
        while (!event2main_.Empty())
        {
            ReleaseMainEvent_(event2main_.Pop());
        }
        std::lock_guard<std::mutex> lock(lock_);
        for (auto* event : overflow_events_)
        {
            ReleaseMainEvent_(event);
        }
        overflow_events_.clear();
        for (auto& conn_ids : overflow_error_conn_ids_)
        {
            conn_ids.clear();
        }
        overflow_events_high_water_warned_ = false;
    }

    void NetworkChannel::ReleaseMainEvent_(NetEventMain* event)
    {
        if (!event)
        {
            return;
        }
        // 持锁或停服路径只回收资源,不派发业务回调
        if (event->GetID() == EID_WorkerToMainAccepting)
        {
            CloseRawFd(event->net_evt_.accepting_.fd_, "accepting not joined");
            event->net_evt_.accepting_.fd_ = -1;
        }
        GIVE_BACK_OBJECT(event);
    }

    void NetworkChannel::CheckOverflowEventsHighWater_()
    {
        if (!overflow_events_high_water_warned_
            && overflow_events_.size() >= OVERFLOW_EVENT_TO_MAIN_HIGH_WATER)
        {
            overflow_events_high_water_warned_ = true;
            NetworkLogWarn("[Network] overflow_events backlog high water. size:%zu water:%d",
                           overflow_events_.size(), OVERFLOW_EVENT_TO_MAIN_HIGH_WATER);
        }
    }

    void NetworkChannel::EmitNetworkErrored_(NetworkType type, uint64_t opaque, uint64_t conn_id,
                                            ENetErrCode err, int32_t err_no)
    {
        OnErrored(type, opaque, conn_id, err, err_no);
        if (errored_)
        {
            errored_(type, opaque, conn_id, err, err_no);
        }
    }

    bool NetworkChannel::Start(std::size_t net_thread_num/* = 1*/)
    {
        // 设置进程可打开的最大文件描述符数量[需要root权限才可成功]
        // SetSystemMaxOpenFiles();

        if (!workers_.empty() || !networks_.empty())
        {
            NetworkLogError("[Network] Start rejected: already started. workers:%zu networks:%zu",
                            workers_.size(), networks_.size());
            return false;
        }
        if (0 == net_thread_num)
        {
            NetworkLogError("[Network] Start rejected: net_thread_num is 0");
            return false;
        }

        stop_.store(false);
        networks_.resize(net_thread_num);
        for (std::size_t i = 0; i < net_thread_num; ++i)
        {
            for (size_t type = static_cast<size_t>(NT_UNKNOWN) + 1;
                 type < static_cast<size_t>(NT_MAX);
                 ++type)
            {
                auto net_type = static_cast<NetworkType>(type);
                if (!IsNetworkTypeSupported_(net_type))
                {
                    continue;
                }
                networks_[i][type].reset(GetNetwork_(net_type, static_cast<uint32_t>(i)));
                if (!networks_[i][type])
                {
                    NetworkLogError("[Network] create network implementation failed. thread:%zu type:%zu",
                                    i, type);
                    RollbackStart_();
                    return false;
                }
            }
        }
        // 校验缓存目标:线程越界可加大线程数再 Start(整表保留);
        // 空事件/不支持类型等不可恢复项剔除后再 Rollback,避免永久卡死 Start
        for (auto it = cached_event_to_worker_.begin(); it != cached_event_to_worker_.end();)
        {
            const uint32_t thread_index = std::get<0>(*it);
            const NetworkType type = std::get<1>(*it);
            NetEventWorker* event = std::get<2>(*it);
            if (thread_index >= networks_.size())
            {
                NetworkLogError("[Network] cached event thread out of range. type:%u thread:%u networks:%zu",
                                type, thread_index, networks_.size());
                EmitNetworkErrored_(type, event ? event->GetOpaque() : 0, 0,
                                    ENetErrCode::NET_INVALID_NET_THREAD_INDEX, 0);
                RollbackStart_();
                return false;
            }
            if (nullptr == event
                || !IsNetworkTypeSupported_(type)
                || !networks_[thread_index][static_cast<size_t>(type)])
            {
                const ENetErrCode err = (nullptr == event)
                                       ? ENetErrCode::NET_INVALID_EVENT
                                       : ENetErrCode::NET_INVALID_NETWORK_TYPE;
                NetworkLogError("[Network] drop unrecoverable cached event. type:%u thread:%u err:%d",
                                type, thread_index, static_cast<int32_t>(err));
                EmitNetworkErrored_(type, event ? event->GetOpaque() : 0, 0, err, 0);
                if (event)
                {
                    GIVE_BACK_OBJECT(event);  // Join 类由析构 CloseRawFd
                }
                it = cached_event_to_worker_.erase(it);
                RollbackStart_();
                return false;
            }
            ++it;
        }

        workers_ready_.store(false);
        try
        {
            workers_.reserve(net_thread_num);
            for (std::size_t i = 0; i < net_thread_num; i++)
            {
                NetworkLogInfo("[Network] start thread. network_thread_index:%zu", i);
                workers_.emplace_back(new std::thread([this, i]()
                {
                    while (!workers_ready_.load() && !stop_.load())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    while (!stop_.load())
                    {
                        std::time_t timetamp = GetMillSecondTimeStamp();
                        bool loaded_network = false;
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
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                    }
                    NetworkLogWarn("[Network] network has stoped. networks_ size:%zu", networks_.size());
                }));
            }
        }
        catch (...)
        {
            NetworkLogError("[Network] create worker thread failed. created:%zu expected:%zu",
                            workers_.size(), net_thread_num);
            RollbackStart_();
            return false;
        }

        // 记录已成功入队项的 (type, opaque):flush 中途失败回滚时,这些事件会随
        // RollbackStart_ 的 UnInit 一并丢弃,须补发 Errored 让业务感知并重试
        std::vector<std::pair<NetworkType, uint64_t>> queued_for_errored;
        queued_for_errored.reserve(cached_event_to_worker_.size());
        for (std::size_t idx = 0; idx < cached_event_to_worker_.size(); ++idx)
        {
            auto& iter = cached_event_to_worker_[idx];
            const uint32_t thread_index = std::get<0>(iter);
            const NetworkType type = std::get<1>(iter);
            NetEventWorker* event = std::get<2>(iter);
            const uint64_t opaque = (nullptr != event) ? event->GetOpaque() : 0;
            // 无论成败,event 的所有权都已交给 NotifyWorker(入队或归还)
            ENetErrCode err = NotifyWorker(event, type, thread_index);
            std::get<2>(iter) = nullptr;
            if (ENetErrCode::NET_SUCCESS != err)
            {
                // 未投递部分[idx+1, end)保留在缓存中,下次 Start 成功即自动补投,保证 resume 闭环
                cached_event_to_worker_.erase(cached_event_to_worker_.begin(),
                                              cached_event_to_worker_.begin() + static_cast<std::ptrdiff_t>(idx + 1));
                NetworkLogError("[Network] flush cached event failed. err:%d type:%u thread:%u kept_for_resume:%zu",
                                static_cast<int32_t>(err), type, thread_index,
                                cached_event_to_worker_.size());
                RollbackStart_();
                // 回滚已拆除网络,对之前成功入队但被丢弃的事件补发 Errored.
                // 用 NET_NOT_STARTED:实际是启动失败/回滚,不是线程号非法
                for (auto& info : queued_for_errored)
                {
                    EmitNetworkErrored_(info.first, info.second, 0,
                                        ENetErrCode::NET_NOT_STARTED, 0);
                }
                return false;
            }
            queued_for_errored.emplace_back(type, opaque);
        }
        cached_event_to_worker_.clear();
        workers_ready_.store(true);
        return true;
    }
    void NetworkChannel::Update()
    {
        if (dispatching_main_events_)
        {
            dispatch_main_events_pending_ = true;
            return;
        }
        dispatching_main_events_ = true;
        dispatching_generation_ = lifecycle_generation_;
        try
        {
            do
            {
                dispatch_main_events_pending_ = false;
                DispatchMainEvent_();
            } while (dispatch_main_events_pending_ && !IsMainDispatchAborted_());
        }
        catch (...)
        {
            dispatching_main_events_ = false;
            throw;
        }
        dispatching_main_events_ = false;
    }

    void NetworkChannel::StopWait()
    {
        ++lifecycle_generation_;
        stop_.store(true);
        for (auto& workder : workers_)
        {
            if (workder)
            {
                workder->join();
                workder.reset();
            }
        }
        workers_.clear();
        workers_ready_.store(false);

        // 停服后只归还,不再派发,避免在已拆网络上 Join/Accept
        DrainMainEvents_();
        // 清空未 Start 期间缓存的待发事件(Join 事件的 fd 由析构兜底关闭)
        for (auto& iter : cached_event_to_worker_)
        {
            GIVE_BACK_OBJECT(std::get<2>(iter));
        }
        cached_event_to_worker_.clear();
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
        networks_.clear();
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
        ENetErrCode err = NotifyWorker(event, iter->second, GetNetThreadIndex(conn_id));
        if (ENetErrCode::NET_SUCCESS != err)
        {
            // 失败保留 conn_type_,便于上层 resume 重试 Close
            return err;
        }
        conn_type_.erase(iter);
        return ENetErrCode::NET_SUCCESS;
    }
    ENetErrCode NetworkChannel::Send(uint64_t conn_id, const char* data, uint32_t size)
    {
        auto iter = conn_type_.find(conn_id);
        if (iter == conn_type_.end())
        {
            NetworkLogError("[network] invalid conn_id:%lu", conn_id);
            return ENetErrCode::NET_INVALID_CONNID;
        }
        auto* data_to_worker = GET_NET_MEMORY(size+sizeof(uint32_t));
        memcpy(data_to_worker, (char*)&size, sizeof(uint32_t));
        memcpy(data_to_worker + sizeof(uint32_t), data, size);
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSend);
        event->SetConnectID(conn_id);
        event->SetData(data_to_worker, size+sizeof(uint32_t));
        return NotifyWorker(event, iter->second, GetNetThreadIndex(conn_id));
    }

    bool NetworkChannel::NotifyMain(NetEventMain* event)
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (!event2main_.Full() && overflow_events_.empty())
        {
            // 多生产者由 lock_ 串行,ring 始终只有一个逻辑写者
            event2main_.Push(std::move(event));
            return true;
        }
        const bool entering_overflow = overflow_events_.empty();
        // overflow 非空后控制事件必须继续追加,禁止回到 ring 越过旧事件
        if (EID_WorkerToMainRecv != event->GetID())
        {
            if (entering_overflow)
            {
                NetworkLogWarn("[Network] event2main enters overflow mode. eid:%d network_type:%u",
                               event->GetID(), event->network_type_);
            }
            overflow_events_.push_back(event);
            CheckOverflowEventsHighWater_();
            return true;
        }
        // 数据事件维持过载丢弃策略,错误事件按原位置追加到 overflow
        if (entering_overflow)
        {
            NetworkLogError("[Network] event2main enters overflow mode, drop recv. network_type:%u",
                            event->network_type_);
        }
        const uint64_t conn_id = event->net_evt_.recv_.connect_id_;
        const size_t type_index = static_cast<size_t>(event->network_type_);
        const bool should_report =
            type_index >= overflow_error_conn_ids_.size()
            || overflow_error_conn_ids_[type_index].insert(conn_id).second;
        if (should_report)
        {
            auto* error_event = GET_NET_OBJECT(
                NetEventMain, EID_WorkerToMainErrored, event->GetOpaque());
            error_event->network_type_ = event->network_type_;
            error_event->net_evt_.error_.connect_id_ = conn_id;
            error_event->net_evt_.error_.net_err_code =
                ENetErrCode::NET_CACHED_EVENT_OVERFLOW;
            error_event->net_evt_.error_.sys_err_code = 0;
            overflow_events_.push_back(error_event);
            CheckOverflowEventsHighWater_();
        }
        GIVE_BACK_OBJECT(event);
        return false;
    }

    void NetworkChannel::SetSimulateNagle(uint32_t packets_num /* = 10*/, uint32_t timeout/* = 2*/)
    {
        for (uint32_t net_index = 0; net_index < networks_.size(); net_index++)
        {
            auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerSetSimulateNagle);
            event->SetFeatureParam(packets_num, timeout);
            ENetErrCode err = NotifyWorker(event, NT_TCP, net_index);
            if (ENetErrCode::NET_SUCCESS != err)
            {
                NetworkLogError("[Network] SetSimulateNagle notify failed. err:%d thread:%u",
                                static_cast<int32_t>(err), net_index);
            }
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
        ENetErrCode err = NotifyWorker(event, type, 0);   // 多网络线程下,第0个线程专门作为 acceptor
        if (ENetErrCode::NET_SUCCESS != err)
        {
            NetworkLogError("[Network] Accept notify failed. err:%d type:%u opaque:%lu %s:%u",
                            static_cast<int32_t>(err), type, opaque, ip.c_str(), port);
        }
    }
    void NetworkChannel::Connect(NetworkType type, uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerNewConnecter, opaque);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        // 线程选择:仅 TCP 的 conn_id 高字节编码线程号,可安全随机分散到多线程;
        // UDP/KCP 的 conn_id 由对端地址派生,GetNetThreadIndex 恒为 0,后续 Send/Close
        // 只会路由到 0 号线程,故其 CONNECTOR 必须落在 0 号线程,否则事件与 socket 错位.
        uint32_t connect_thread_index = 0;
        if (NT_TCP == type && networks_.size() > 1)
        {
            static std::default_random_engine dre(time(0));
            std::uniform_int_distribution<unsigned> uid(0, networks_.size() - 1);
            connect_thread_index = uid(dre);
        }
        NetworkLogInfo("[Network] Push connect event to network_thread_index:%u", connect_thread_index);
        ENetErrCode err = NotifyWorker(event, type, connect_thread_index);
        if (ENetErrCode::NET_SUCCESS != err)
        {
            NetworkLogError("[Network] Connect notify failed. err:%d type:%u opaque:%lu %s:%u thread:%u",
                            static_cast<int32_t>(err), type, opaque, ip.c_str(), port, connect_thread_index);
        }
    }
    ENetErrCode NetworkChannel::NotifyWorker(NetEventWorker* event, NetworkType type, uint32_t net_thread_index)
    {
        if (nullptr == event)
        {
            NetworkLogError("[Network] NotifyWorker event is null. type:%u thread:%u", type, net_thread_index);
            EmitNetworkErrored_(type, 0, 0, ENetErrCode::NET_INVALID_EVENT, 0);
            return ENetErrCode::NET_INVALID_EVENT;
        }
        // 失败分支会归还 event,故先取出错误上报所需的定位信息
        const EventID event_id = event->GetID();
        const uint64_t opaque = event->GetOpaque();
        // 只有 Send/Close 事件的 union 成员承载有效 conn_id,其余事件不可读
        const uint64_t conn_id = (EID_MainToWorkerSend == event_id || EID_MainToWorkerClose == event_id)
                                 ? event->GetConnectID() : 0;

        if (type >= NT_MAX || type <= NT_UNKNOWN )
        {
            NetworkLogError("[Network] NotifyWorker invalid network type:%u", type);
            EmitNetworkErrored_(type, opaque, conn_id, ENetErrCode::NET_INVALID_NETWORK_TYPE, 0);
            GIVE_BACK_OBJECT(event);
            return ENetErrCode::NET_INVALID_NETWORK_TYPE;
        }

        if (0 == networks_.size())
        {
            if (cached_event_to_worker_.size() >= NUM_OF_CACHED_EVENT_TO_WORKER)
            {
                NetworkLogError("[Network] Vector of cache event to net thread is overflow. network_thread_index:%u", net_thread_index);
                EmitNetworkErrored_(type, opaque, conn_id, ENetErrCode::NET_CACHED_EVENT_OVERFLOW, 0);
                GIVE_BACK_OBJECT(event);
                return ENetErrCode::NET_CACHED_EVENT_OVERFLOW;
            }
            NetworkLogInfo("[Network] Cache event for net thread. network_thread_index:%u", net_thread_index);
            cached_event_to_worker_.emplace_back(std::make_tuple(net_thread_index, type, event));
            return ENetErrCode::NET_SUCCESS;
        }

        if (net_thread_index >= networks_.size())
        {
            NetworkLogError("[Network] NotifyWorker net_thread_index:%u, networks_.size():%zu", net_thread_index, networks_.size());
            EmitNetworkErrored_(type, opaque, conn_id, ENetErrCode::NET_INVALID_NET_THREAD_INDEX, 0);
            GIVE_BACK_OBJECT(event);
            return ENetErrCode::NET_INVALID_NET_THREAD_INDEX;
        }

        auto& network_type = networks_[net_thread_index];

        auto index = static_cast<size_t>(type);
        if (nullptr == network_type[index])
        {
            NetworkLogError("[Network] network implementation is null. thread:%u type:%zu",
                            net_thread_index, index);
            EmitNetworkErrored_(type, opaque, conn_id, ENetErrCode::NET_INVALID_NETWORK_TYPE, 0);
            GIVE_BACK_OBJECT(event);
            return ENetErrCode::NET_INVALID_NETWORK_TYPE;
        }
        if (!network_type[index]->PushEvent(std::move(event)))
        {
            // PushEvent 内已归还 event
            NetworkLogError("[Network] worker event queue full. thread:%u type:%zu eid:%d",
                            net_thread_index, index, event_id);
            EmitNetworkErrored_(type, opaque, conn_id, ENetErrCode::NET_CACHED_EVENT_OVERFLOW, 0);
            return ENetErrCode::NET_CACHED_EVENT_OVERFLOW;
        }
        return ENetErrCode::NET_SUCCESS;
    }
    void NetworkChannel::OnWorkerToMainBinded_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        NetworkLogTrace("[Network] OnWorkerToMainBinded_. network_type:%u, connid:%lu", event_main->network_type_, event_main->net_evt_.accept_.connect_id_);
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.bind_.connect_id_] = event_main->network_type_;
        OnBinded(event_main->network_type_
                 , event_main->GetOpaque()
                 , event_main->net_evt_.bind_.connect_id_
                 , event_main->GetBindIP()
                 , event_main->net_evt_.bind_.port_);
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
            NetworkLogError("[network] event dynamic_cast to NetEventMain failed");
            return;
        }
        const int32_t fd = event_main->net_evt_.accepting_.fd_;
        const NetworkType network_type = event_main->network_type_;
        // 注意:此处不提前把事件 fd 置 -1.回调可能抛异常,若提前清零,
        // 外层 catch 的 ReleaseMainEvent_ 将拿不到 fd 而泄漏.保持事件持有 fd,
        // 直到 Join 接管 或 本函数显式关闭后再清零.
        OnAccepting(network_type, event_main->GetOpaque(), fd);
        if (accepting_)
        {
            accepting_(network_type, event_main->GetOpaque(), fd);
        }
        NetworkLogTrace("[Network] OnWorkerToMainAccepting_. network_type:%u, fd:%d", network_type, fd);
        // 回调内可能已 StopWait,此时网络实例已拆除,不再 Join,直接关闭 fd
        if (IsMainDispatchAborted_())
        {
            NetworkLogWarn("[Network] network stopped in OnAccepting callback, drop fd:%d", fd);
            CloseRawFd(fd, "accepting aborted");
            event_main->net_evt_.accepting_.fd_ = -1;
            return;
        }
        JoinIOMultiplexing(network_type, event_main->GetOpaque(), fd
                           , event_main->GetAcceptingIP()
                           , event_main->net_evt_.accepting_.port_
                           , event_main->net_evt_.accepting_.send_buff_size_
                           , event_main->net_evt_.accepting_.recv_buff_size_);
        // Join 已接管 fd 所有权(失败亦由 ~NetEventWorker 兜底关闭),清事件持有避免二次关闭
        event_main->net_evt_.accepting_.fd_ = -1;
    }

    void NetworkChannel::OnWorkerToMainAccepted_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        NetworkLogTrace("[Network] OnWorkerToMainAccepted_. network_type:%u, connid:%lu", event_main->network_type_, event_main->net_evt_.accept_.connect_id_);
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.accept_.connect_id_] = event_main->network_type_;
        OnAccepted(event_main->network_type_
                   , event_main->GetOpaque()
                   , event_main->net_evt_.accept_.connect_id_);
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
        // 先删映射再回调:Closed 语义为"连接已结束",回调内对本 conn 的 Send/Close
        // 应返回 NET_INVALID_CONNID,避免在已关闭连接上重复投递
        conn_type_.erase(event_main->net_evt_.close_.connect_id_);
        OnClose(event_main->network_type_
                , event_main->GetOpaque()
                , event_main->net_evt_.close_.connect_id_
                , event_main->net_evt_.close_.net_err_
                , event_main->net_evt_.close_.sys_err_);
        if (close_)
        {
            close_(event_main->network_type_
                   , event_main->GetOpaque()
                   , event_main->net_evt_.close_.connect_id_
                   , event_main->net_evt_.close_.net_err_
                   , event_main->net_evt_.close_.sys_err_);
        }
    }

    void NetworkChannel::OnWorkerToMainConnected_(Event* event)
    {
        auto event_main = dynamic_cast<NetEventMain*>(event);
        if (nullptr == event_main)
        {
            return;
        }
        // 建立 conn_id 到 network_type 的映射
        conn_type_[event_main->net_evt_.connect_sucessed_.connect_id_] = event_main->network_type_;
        OnConnected(event_main->network_type_
                    , event_main->GetOpaque()
                    , event_main->net_evt_.connect_sucessed_.connect_id_);
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
        EmitNetworkErrored_(event_main->network_type_
                            , event_main->GetOpaque()
                            , event_main->net_evt_.error_.connect_id_
                            , event_main->net_evt_.error_.net_err_code
                            , event_main->net_evt_.error_.sys_err_code);
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



    bool NetworkChannel::IsMainDispatchAborted_() const
    {
        return dispatching_generation_ != lifecycle_generation_;
    }

    void NetworkChannel::DispatchMainEvent_()
    {
        while (!event2main_.Empty())
        {
            NetEventMain* event = event2main_.Pop();
            if (nullptr == event)
            {
                EmitNetworkErrored_(NT_UNKNOWN, 0, 0, ENetErrCode::NET_INVALID_EVENT, 0);
                continue;
            }
            // 上一个回调内已 StopWait,网络实例已拆除,剩余事件只回收不派发
            if (IsMainDispatchAborted_())
            {
                ReleaseMainEvent_(event);
                continue;
            }
            try
            {
                event_dispatcher_->HandleEvent(event);
            }
            catch (...)
            {
                NetworkLogError("[Network] main event callback threw exception. eid:%d",
                                event->GetID());
                ReleaseMainEvent_(event);
                continue;
            }
            GIVE_BACK_OBJECT(event);
        }
        if (IsMainDispatchAborted_())
        {
            NetworkLogWarn("[Network] main dispatch aborted by StopWait, skip overflow events");
            return;
        }
        DispatchOverflowEvents_();
    }

    void NetworkChannel::DispatchOverflowEvents_()
    {
        std::vector<NetEventMain*> overflow_events;
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (overflow_events_.empty())
            {
                return;
            }
            // overflow 非空期间生产者不会回写 ring,因此当前批次严格晚于已排空事件
            overflow_events.swap(overflow_events_);
            for (auto& conn_ids : overflow_error_conn_ids_)
            {
                conn_ids.clear();
            }
            overflow_events_high_water_warned_ = false;
        }
        NetworkLogInfo("[Network] event2main leaves overflow mode. dispatch_size:%zu",
                       overflow_events.size());
        for (auto* event : overflow_events)
        {
            if (nullptr == event)
            {
                EmitNetworkErrored_(NT_UNKNOWN, 0, 0, ENetErrCode::NET_INVALID_EVENT, 0);
                continue;
            }
            // 本地批次已脱离 overflow_events_,StopWait 无法回收,只能在此逐个释放
            if (IsMainDispatchAborted_())
            {
                ReleaseMainEvent_(event);
                continue;
            }
            try
            {
                event_dispatcher_->HandleEvent(event);
            }
            catch (...)
            {
                NetworkLogError("[Network] overflow event callback threw exception. eid:%d",
                                event->GetID());
                ReleaseMainEvent_(event);
                continue;
            }
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
                if (!tcp_network->Init(this, type, net_thread_index))
                {
                    NetworkLogError("[Network] init TCP network failed. thread:%u", net_thread_index);
                    tcp_network->UnInit();
                    delete tcp_network;
                    return nullptr;
                }
                return tcp_network;
            }
            case NT_UDP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                if (!udp_network->Init(this, type, net_thread_index))
                {
                    NetworkLogError("[Network] init UDP network failed. thread:%u", net_thread_index);
                    udp_network->UnInit();
                    delete udp_network;
                    return nullptr;
                }
                return udp_network;
#endif // __linux__
                break;
            }
            case NT_KCP:
            {
#if defined(__linux__)
                auto* udp_network = new UdpEpollNetwork();
                if (!udp_network->Init(this, type, net_thread_index))
                {
                    NetworkLogError("[Network] init KCP network failed. thread:%u", net_thread_index);
                    udp_network->UnInit();
                    delete udp_network;
                    return nullptr;
                }
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
        // 解码线程号,编码方式见 socket_pool.h MakeUint32.
        // 仅对 TCP 的 pool conn_id 有效;UDP/KCP 的 conn_id 由地址派生(ip<<32|port),
        // bit16~23 恒为 0,故其 socket 必须驻留 0 号线程(见 Connect 的线程选择).
        return (conn_id >> 16) & 0x00FF;
    }

    void NetworkChannel::JoinIOMultiplexing(NetworkType type, uint64_t opaque, int32_t fd, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto* event = GET_NET_OBJECT(NetEventWorker, EID_MainToWorkerJoinIOMultiplexing, opaque);
        event->SetFd(fd);
        event->SetIP(ip);
        event->SetAddressPort(port);
        event->SetBuffSize(send_buff_size, recv_buff_size);
        if (networks_.empty())
        {
            NetworkLogError("[Network] join multiplexing failed: no network worker. opaque:%lu fd:%d",
                            opaque, fd);
            // fd 由事件析构兜底关闭
            GIVE_BACK_OBJECT(event);
            EmitNetworkErrored_(type, opaque, 0, ENetErrCode::NET_NOT_STARTED, 0);
            return;
        }
        static std::default_random_engine dre(time(0));
        std::uniform_int_distribution<unsigned> uid(0, networks_.size() - 1);
        uint32_t random_thread_index = uid(dre);
        ENetErrCode err = NotifyWorker(event, type, random_thread_index);
        if (ENetErrCode::NET_SUCCESS != err)
        {
            // NotifyWorker 失败已 EmitNetworkErrored_ 并归还 event(析构兜底关 fd)
            NetworkLogError("[Network] join multiplexing notify failed. err:%d type:%u opaque:%lu fd:%d thread:%u",
                            static_cast<int32_t>(err), type, opaque, fd, random_thread_index);
        }
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