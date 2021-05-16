#include "event.h"
#include "tools/memory_pool.h"
#include <string.h>

Event::Event(uint32_t id)
    : id_(id)
    {
        
    }

uint32_t Event::GetID()
{
    return id_;
}



NetEventWorker::NetEventWorker(NetEventWorkerType type)
    :type_(type)
{
}

NetEventWorker::~NetEventWorker()
{
    switch(type_)
    {
        case MainToWorkerSend:
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







NetEventMain::NetEventMain(NetEvenMainType type)
{
    type_ = type;
}
NetEventMain::~NetEventMain()
{
    switch(type_)
    {
        case WorkerToMainRecv:
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
void EventBasedObject::RegistereventHandler(uint32_t event_id, EventHandle func)
{
    event_func_map_[event_id] = func;
}
void EventBasedObject::UnregisterEventHandler(uint32_t event_id)
{
    auto iter = event_func_map_.find(event_id);
    if(iter != event_func_map_.end())
    {
        event_func_map_.erase(event_id);
    }
}