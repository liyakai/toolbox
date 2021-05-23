#include "network.h"
#include "tools/object_pool.h"
#include <functional>

INetwork::INetwork()
{
    RegistereventHandler(EID_MainToWorkerNewAccepter,std::bind(&INetwork::OnMainToWorkerNewAccepter_,this, std::placeholders::_1));
    RegistereventHandler(EID_MainToWorkerNewConnecter,std::bind(&INetwork::OnMainToWorkerNewConnecter_,this, std::placeholders::_1));
    RegistereventHandler(EID_MainToWorkerSend,std::bind(&INetwork::OnMainToWorkerSend_,this, std::placeholders::_1));
    RegistereventHandler(EID_MainToWorkerClose,std::bind(&INetwork::OnMainToWorkerClose_,this, std::placeholders::_1));

}

INetwork::~INetwork()
{
    while (!event2worker_.Empty())
    {
        NetEventWorker* event;
        if(event2worker_.Read<NetEventWorker*>(event))
        {
            MemPoolMgr->GiveBack((char*)event);
        }
        
    }
}

void INetwork::Init(NetworkMaster* master)
{
    master_ = master;
}
void INetwork::Update()
{
    // 处理主线程发来的事件
    HandleEvents_();
}

void INetwork::PushEvent(NetEventWorker* event)
{
    event2worker_.Write<NetEventWorker*>(event);
}

void INetwork::OnClose(uint64_t connect_id)
{
    auto* close_event = GetObject<NetEventMain>(EID_WorkerToMainClose);
    close_event->SetConnectID(connect_id);
    master_->NotifyMain(close_event);
}

void INetwork::OnReceived(uint64_t connect_id, const char* data, uint32_t size)
{
    auto* receive_event = GetObject<NetEventMain>(EID_WorkerToMainRecv);
    receive_event->SetData(data, size);
    receive_event->SetConnectID(connect_id);
    master_->NotifyMain(receive_event);
}

void INetwork::HandleEvents_()
{
    while(!event2worker_.Empty())
    {
        NetEventWorker* event = nullptr;
        if(event2worker_.Read<NetEventWorker*>(event))
        {
            HandleEvent(event);
            MemPoolMgr->GiveBack((char*)event);
        }
    }
}