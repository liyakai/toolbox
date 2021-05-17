#include "network/network_mgr.h"

NetworkMaster::NetworkMaster()
{
    stop_.store(false);
}

NetworkMaster::~NetworkMaster()
{}

bool NetworkMaster::Start()
{
    stop_.store(false);
    worker_.reset(new std::thread([this](){
        while(!stop_.load())
        {
            // 驱动网络更新
            for(auto& network : networks_)
            {
                if(network)
                {
                    network->Update();
                }
            }
        }
    }));
    return true;
}
void NetworkMaster::Update()
{
    DispatchMainEvent_();
}

void NetworkMaster::StopWait()
{
    stop_.store(true);
    if(worker_)
    {
        worker_->join();
        worker_.reset();
    }
    while (!event2main_.Empty())
    {
        NetEventWorker* event;
        if(event2main_.Read<NetEventWorker*>(event))
        {
            MemPoolMgr->GiveBack((char*)event);
        }
    }
    for(auto& network : networks_)
    {
        network.reset();
    }

}
void NetworkMaster::Close(NetworkType type, uint64_t conn_id)
{

}
void NetworkMaster::Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size) 
{

}

void NetworkMaster::Accept(const std::string& ip, uint16_t port, NetworkType type)
{
    auto* event = new NetEventWorker(MainToWorkerNewAccepter);
    event->SetIP(ip);
    event->SetPort(port);
    NotifyWorker(event,type);
}
void NetworkMaster::Connect(const std::string& ip, uint16_t port, NetworkType type)
{

}
void NetworkMaster::NotifyWorker(NetEventWorker* event, NetworkType type)
{

}
void NetworkMaster::NotifyMain(NetEventMain* event)
{

}



void NetworkMaster::DispatchMainEvent_()
{
    while(!event2main_.Empty())
    {
        NetEventMain* event;
        if(!event2main_.Read<NetEventMain*>(event))
        {
            continue;
        }
        switch (event->GetType())
        {
        case WorkerToMainBinded:
            break;
        case WorkerToMainAccepted:
            OnAccepted(event->GetConnectID());
            break;
        case WorkerToMainClose:
            OnClose(event->GetConnectID());
            break;
        case WorkerToMainConnected:
            OnConnected(event->GetConnectID());
            break;
        case WorkerToMainRecv:
            OnReceived(event->GetConnectID(), event->GetData(), event->GetDataSize());
            break;
        default:
            break;
        }
    }
}
