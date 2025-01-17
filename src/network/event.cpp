#include "event.h"
#include "network_def_internal.h"
#include <string.h>
#include <cstddef>
#include <string.h>

namespace ToolBox
{

    Event::Event(EventID id, uint64_t opaque/* = 0 */)
        : id_(id), opaque_(opaque)
    {

    }
    Event::~Event()
    {
      id_ = EID_NONE; // 事件ID
      opaque_ = 0;    // 信道标记
    }
    EventID Event::GetID()
    {
        return id_;
    }
    uint64_t Event::GetOpaque()
    {
        return opaque_;
    }


    NetEventWorker::NetEventWorker(EventID event_id, uint64_t opaque/* = 0 */)
        : Event(event_id, opaque)
    {
    }

    NetEventWorker::~NetEventWorker()
    {
        switch (GetID())
        {
            case EID_MainToWorkerSend:
                GIVE_BACK_MEMORY_RAW(net_req_.stream_.data_, "NetEventWorker::~NetEventWorker");
                break;
            default:
                break;
        }
    }


    void NetEventWorker::SetIP(const std::string& ip)
    {
        memset(net_req_.address_.ip_, 0, sizeof(net_req_.address_.ip_));
        for (std::size_t idx = 0; idx < sizeof(net_req_.address_.ip_) && idx < ip.size(); idx++)
        {
            net_req_.address_.ip_[idx] = ip[idx];
        }
    }

    std::string NetEventWorker::GetIP() const
    {
        return net_req_.address_.ip_;
    }

    void NetEventWorker::SetAddressPort(const uint16_t port)
    {
        net_req_.address_.port_ = port;
    }

    uint16_t NetEventWorker::GetPort() const
    {
        return net_req_.address_.port_;
    }
    void NetEventWorker::SetBuffSize(int32_t send_size, int32_t recv_size)
    {
        net_req_.address_.send_buff_size = send_size;
        net_req_.address_.recv_buff_size = recv_size;
    }

    int32_t NetEventWorker::GetSendBuffSize()
    {
        return net_req_.address_.send_buff_size;
    }

    int32_t NetEventWorker::GetRecvBuffSize()
    {
        return net_req_.address_.recv_buff_size;
    }

    void NetEventWorker::SetConnectID(const uint64_t conn_id)
    {
        net_req_.stream_.connect_id_ = conn_id;
    }
    uint64_t NetEventWorker::GetConnectID() const
    {
        return net_req_.stream_.connect_id_;
    }

    void NetEventWorker::SetData(const char* data, uint32_t size)
    {
        net_req_.stream_.data_ = const_cast<char*>(data);
        net_req_.stream_.size_ = size;
    }
    char* NetEventWorker::GetData() const
    {
        return net_req_.stream_.data_;
    }
    uint32_t NetEventWorker::GetDataSize() const
    {
        return net_req_.stream_.size_;
    }

    void NetEventWorker::SetFd(int32_t fd)
    {
        net_req_.address_.fd_ = fd;
    }

    int32_t NetEventWorker::GetFd() const
    {
        return net_req_.address_.fd_;
    }

    void NetEventWorker::SetFeatureParam(int32_t param1, int32_t param2)
    {
        net_req_.net_feature_.param1_ = param1;
        net_req_.net_feature_.param2_ = param2;
    }

    std::tuple<int32_t, int32_t> NetEventWorker::GetFeatureParam()
    {
        return std::tuple<int32_t, int32_t> {net_req_.net_feature_.param1_, net_req_.net_feature_.param2_};
    }

    NetEventMain::NetEventMain(EventID event_id, uint64_t opaque/* = 0 */)
        : Event(event_id, opaque)
    {
    }
    NetEventMain::~NetEventMain()
    {
        switch (GetID())
        {
            case EID_WorkerToMainRecv: 
                GIVE_BACK_MEMORY_RAW((char*)net_evt_.recv_.data_);
                break;
            default:
                break;
        }
    }
    void NetEventMain::SetBindIP(const std::string& ip)
    {
        memset(net_evt_.bind_.ip_, 0, sizeof(net_evt_.bind_.ip_));
        for (std::size_t idx = 0; idx < sizeof(net_evt_.bind_.ip_) && idx < ip.size(); idx++)
        {
            net_evt_.bind_.ip_[idx] = ip[idx];
        }
    }

    std::string NetEventMain::GetBindIP() const
    {
        return net_evt_.bind_.ip_;
    }
    void NetEventMain::SetAcceptingIP(const std::string& ip)
    {
        memset(net_evt_.accepting_.ip_, 0, sizeof(net_evt_.accepting_.ip_));
        for (std::size_t idx = 0; idx < sizeof(net_evt_.accepting_.ip_) && idx < ip.size(); idx++)
        {
            net_evt_.accepting_.ip_[idx] = ip[idx];
        }
    }

    std::string NetEventMain::GetAcceptingIP() const
    {
        return net_evt_.accepting_.ip_;
    }

    EventDispatcher::EventDispatcher()
    {
    }

    EventDispatcher:: ~EventDispatcher()
    {

    }

    void EventDispatcher::HandleEvent(Event* event)
    {
        EventHandle& func = event_func_array_[event->GetID()];
        if (nullptr != func)
        {
            func(event);
        }
    }
    void EventDispatcher::RegistereventHandler(EventID event_id, EventHandle func)
    {
        event_func_array_[event_id] = func;

    }
    void EventDispatcher::UnregisterEventHandler(EventID event_id)
    {
    }

};  // ToolBox