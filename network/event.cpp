#include "event.h"
#include "tools/memory_pool.h"
#include <string.h>

Event::Event(EventID id)
    : id_(id)
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
            MemPoolMgr->GiveBack(detail_.stream_.data_);
            break;
        default:
            break;
    }
}


void NetEventWorker::SetIP(const std::string& ip)
{
    detail_.address_.ip_ = ip;
}

std::string NetEventWorker::GetIP() const
{
    return detail_.address_.ip_;
}

void NetEventWorker::SetPort(const uint16_t port)
{
    detail_.address_.port_ = port;
}

uint16_t NetEventWorker::GetPort() const
{
    return detail_.address_.port_;
}

void NetEventWorker::SetConnectID(const uint64_t conn_id)
{
    detail_.stream_.connect_id_ = conn_id;
}
uint64_t NetEventWorker::GetConnectID() const
{
    return detail_.stream_.connect_id_;
}

void NetEventWorker::SetData(const char* data, uint32_t size)
{
    detail_.stream_.data_ = MemPoolMgr->GetMemory(size);
    detail_.stream_.size_ = size;
    memmove(detail_.stream_.data_, data, size);
}
const char* NetEventWorker::GetData() const
{
    return detail_.stream_.data_;
}
uint32_t NetEventWorker::GetDataSize() const
{
    return detail_.stream_.size_;
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
            MemPoolMgr->GiveBack(stream_.data_);
            break;
        default:
            break;
    }
}

void NetEventMain::SetConnectID(const uint64_t conn_id)
{
    stream_.connect_id_ = conn_id;
}
uint64_t NetEventMain::GetConnectID() const
{
    return stream_.connect_id_;
}

void NetEventMain::SetData(const char* data, uint32_t size)
{
    stream_.data_ = MemPoolMgr->GetMemory(size);
    stream_.size_ = size;
    memmove(stream_.data_, data, size);
}
const char* NetEventMain::GetData() const
{
    return stream_.data_;
}
uint32_t NetEventMain::GetDataSize() const
{
    return stream_.size_;
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