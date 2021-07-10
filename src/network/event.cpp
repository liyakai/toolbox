#include "event.h"
#include "src/tools/memory_pool.h"
#include "src/tools/object_pool.h"
#include <string.h>

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
    switch(GetID())
    {
        case EID_MainToWorkerSend:
            MemPoolMgr->GiveBack(net_req_.stream_.data_);
            break;
        default:
            break;
    }
}


void NetEventWorker::SetIP(const std::string& ip)
{
    net_req_.address_.ip_ = GetObject<std::string>(ip);
}

std::string NetEventWorker::GetIP() const
{
    const static std::string NullString;
    if(net_req_.address_.ip_)
    {
        return *net_req_.address_.ip_;
    }
    return NullString;
}

void NetEventWorker::SetPort(const uint16_t port)
{
    net_req_.address_.port_ = port;
}

uint16_t NetEventWorker::GetPort() const
{
    return net_req_.address_.port_;
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
    net_req_.stream_.data_ = MemPoolMgr->GetMemory(size);
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







NetEventMain::NetEventMain(EventID event_id)
    : Event(event_id)
{
}
NetEventMain::~NetEventMain()
{
    switch(GetID())
    {
        case EID_WorkerToMainRecv:
            MemPoolMgr->GiveBack((char*)net_evt_.recv_.data_);
            break;
        default:
            break;
    }
}

// void NetEventMain::SetConnectID(const uint64_t conn_id)
// {
//     net_evt_.stream_.connect_id_ = conn_id;
// }
// uint64_t NetEventMain::GetConnectID() const
// {
//     return net_evt_.stream_.connect_id_;
// }

// void NetEventMain::SetData(const char* data, uint32_t size)
// {
//     net_evt_.stream_.data_ = MemPoolMgr->GetMemory(size);
//     net_evt_.stream_.size_ = size;
//     memmove(net_evt_.stream_.data_, data, size);
// }
// char* NetEventMain::GetData() const
// {
//     return net_evt_.stream_.data_;
// }
// uint32_t NetEventMain::GetDataSize() const
// {
//     return net_evt_.stream_.size_;
// }

// void NetEventMain::SetConnectFailed(ENetErrCode net_err, int32_t sys_err)
// {
//     net_evt_.connect_failed_.net_err_code = net_err;
//     net_evt_.connect_failed_.sys_err_code = sys_err;
// }

// ENetErrCode NetEventMain::GetNetErr()
// {
//     return net_evt_.connect_failed_.net_err_code;
// }
// int32_t NetEventMain::GetSysErr()
// {
//     return net_evt_.connect_failed_.sys_err_code;
// }

EventBasedObject::EventBasedObject()
{
}

EventBasedObject:: ~EventBasedObject()
{

}

void EventBasedObject::HandleEvent(Event* event)
{
    auto iter = event_func_map_.find(event->GetID());
    if(iter == event_func_map_.end())
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
    if(iter != event_func_map_.end())
    {
        event_func_map_.erase(event_id);
    }
}