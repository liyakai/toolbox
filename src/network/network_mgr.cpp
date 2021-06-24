#include "src/network/network_mgr.h"

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
    auto* event = new NetEventWorker(EID_MainToWorkerClose);
    event->SetConnectID(conn_id);
    NotifyWorker(event,type);
}
void NetworkMaster::Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size) 
{
    auto* event = new NetEventWorker(EID_MainToWorkerSend);
    event->SetConnectID(conn_id);
    event->SetData(data, size);
    NotifyWorker(event, type);
}

void NetworkMaster::Accept(const std::string& ip, uint16_t port, NetworkType type)
{
    auto* event = new NetEventWorker(EID_MainToWorkerNewAccepter);
    event->SetIP(ip);
    event->SetPort(port);
    NotifyWorker(event,type);
}
void NetworkMaster::Connect(const std::string& ip, uint16_t port, NetworkType type)
{
    auto* event = new NetEventWorker(EID_MainToWorkerNewConnecter);
    event->SetIP(ip);
    event->SetPort(port);
    NotifyWorker(event, type);
}
void NetworkMaster::NotifyWorker(NetEventWorker* event, NetworkType type)
{
    auto index = static_cast<size_t>(type);
    if(nullptr == networks_[index])
    {
        networks_[index].reset(GetNetwork_(type));
        networks_[index]->Init(this);
    }
    networks_[index]->PushEvent(std::move(event));
}
void NetworkMaster::NotifyMain(NetEventMain* event)
{
    event2main_.Write<NetEventMain*>(event);
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
        switch (event->GetID())
        {
        case EID_WorkerToMainBinded:
            break;
        case EID_WorkerToMainAccepted:
            OnAccepted(event->GetConnectID());
            break;
        case EID_WorkerToMainClose:
            OnClose(event->GetConnectID());
            break;
        case EID_WorkerToMainConnected:
            OnConnected(event->GetConnectID());
            break;
        case EID_WorkerToMainRecv:
            OnReceived(event->GetConnectID(), event->GetData(), event->GetDataSize());
            break;
        default:
            break;
        }
    }
}

INetwork* NetworkMaster::GetNetwork_(NetworkType type)
{
    switch (type)
    {
    case NT_TCP:
        // return new INetwork(new TcpNetwork());
        break;
    
    default:
        break;
    }
    return nullptr;
}
