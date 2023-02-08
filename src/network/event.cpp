#include "event.h"
#include "network_def.h"
#include <string.h>

namespace ToolBox
{

    Event::Event(EventID id)
        : id_(id)
    {

    }
    Event::~Event()
    {

    }
    EventID Event::GetID()
    {
        return id_;
    }



    NetEventWorker::NetEventWorker(EventID event_id)
        : Event(event_id)
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
        net_req_.address_.ip_ = GetObjectLockFree<std::string>(ip);
    }

    std::string NetEventWorker::GetIP() const
    {
        std::string result;
        if (net_req_.address_.ip_)
        {
            result = *net_req_.address_.ip_;
            GiveBackObjectLockFree(net_req_.address_.ip_);
        }
        return result;
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
        memmove(net_req_.stream_.data_, data, size);
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
        net_req_.fd_ = fd;
    }

    int32_t NetEventWorker::GetFd() const
    {
        return net_req_.fd_;
    }
    void NetEventWorker::SetNetworkType(NetworkType network_type)
    {
        network_type_ = network_type;
    }

    NetworkType NetEventWorker::GetNetworkType() const
    {
        return network_type_;
    }






    NetEventMain::NetEventMain(EventID event_id)
        : Event(event_id)
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

    void NetEventMain::SetIP(const std::string& ip)
    {
        net_evt_.acceptting_.ip_ = GetObjectLockFree<std::string>(ip);
    }

    std::string NetEventMain::GetIP() const
    {
        std::string result;
        if (net_evt_.acceptting_.ip_)
        {
            result = *net_evt_.acceptting_.ip_;
            GiveBackObjectLockFree(net_evt_.acceptting_.ip_);
        }
        return result;
    }

    EventBasedObject::EventBasedObject()
    {
    }

    EventBasedObject:: ~EventBasedObject()
    {

    }

    void EventBasedObject::HandleEvent(Event* event)
    {
        auto iter = event_func_map_.find(event->GetID());
        if (iter == event_func_map_.end())
        {
            return;
        }
        iter->second(event);
    }
    void EventBasedObject::RegistereventHandler(EventID event_id, EventHandle func)
    {
        event_func_map_[event_id] = func;
    }
    void EventBasedObject::UnregisterEventHandler(EventID event_id)
    {
        auto iter = event_func_map_.find(event_id);
        if (iter != event_func_map_.end())
        {
            event_func_map_.erase(event_id);
        }
    }

};  // ToolBox