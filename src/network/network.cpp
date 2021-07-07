#include "network.h"
#include "src/tools/object_pool.h"
#include "event.h"
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
void INetwork::UnInit()
{


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

void INetwork::OnAccepted(uint64_t connect_id)
{
    auto* accept_event = GetObject<NetEventMain>(EID_WorkerToMainAccepted);
    accept_event->SetConnectID(connect_id);
    master_->NotifyMain(accept_event);
}
void INetwork::OnConnected(uint64_t connect_id)
{
    auto* connected_event = GetObject<NetEventMain>(EID_WorkerToMainConnected);
    connected_event->SetConnectID(connect_id);
    master_->NotifyMain(connected_event);
}

void INetwork::OnConnectedFailed(ENetErrCode err_code, int32_t err_no)
{
    auto* connected_failed_event = GetObject<NetEventMain>(EID_WorkerToMainConnected);
    connected_failed_event->SetConnectFailed(err_code, err_no);
    master_->NotifyMain(connected_failed_event);
}

void INetwork::OnClosed(uint64_t connect_id)
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

void INetwork::OnMainToWorkerNewAccepter_(Event* event)
{
    auto accepter_event = dynamic_cast<NetEventWorker*>(event);
    if(nullptr == accepter_event)
    {
        return;
    }
    auto conn_id = OnNewAccepter(accepter_event->GetIP(), accepter_event->GetPort());
    auto bind_tcp = GetObject<NetEventMain>(EID_WorkerToMainBinded);
    bind_tcp->SetConnectID(conn_id);
    master_->NotifyMain(bind_tcp);
}

void INetwork::OnMainToWorkerNewConnecter_(Event* event)
{
    auto connecter_tcp = dynamic_cast<NetEventWorker*>(event);
    if(nullptr == connecter_tcp)
    {
        return;
    }
    OnNewConnecter(connecter_tcp->GetIP(), connecter_tcp->GetPort());
}

void INetwork::OnMainToWorkerClose_(Event* event)
{
    auto close_tcp = dynamic_cast<NetEventWorker*>(event);
    if(nullptr == close_tcp)
    {
        return;
    }
    OnClose(close_tcp->GetConnectID());
}

void INetwork::OnMainToWorkerSend_(Event* event)
{
    auto send_tcp = dynamic_cast<NetEventWorker*>(event);
    if(nullptr == send_tcp)
    {
        return;
    }
    OnSend(send_tcp->GetConnectID(), send_tcp->GetData(), send_tcp->GetDataSize());
}

void INetwork::HandleEvents_()
{
    while(!event2worker_.Empty())
    {
        NetEventWorker* event = nullptr;
        if(event2worker_.Read<NetEventWorker*>(event))
        {
            HandleEvent(event);
            GiveBackObject(event);
        }
    }
}