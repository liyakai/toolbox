#include "network.h"
#include <functional>

INetwork::INetwork()
{
    RegistereventHandler(MainToWorkerNewAccepter,std::bind(&INetwork::OnMainToWorkerNewAccepter_,this, std::placeholders::_1));
    RegistereventHandler(MainToWorkerNewConnecter,std::bind(&INetwork::OnMainToWorkerNewConnecter_,this, std::placeholders::_1));
    RegistereventHandler(MainToWorkerSend,std::bind(&INetwork::OnMainToWorkerSend_,this, std::placeholders::_1));
    RegistereventHandler(MainToWorkerClose,std::bind(&INetwork::OnMainToWorkerClose_,this, std::placeholders::_1));

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
void INetwork::Update()
{
    // �������̷߳������¼�
    HandleEvents_();
}


void INetwork::HandleEvents_()
{
    while(!event2worker_.Empty())
    {
        NetEventWorker* event;
        if(event2worker_.Read<NetEventWorker*>(event))
        {
            MemPoolMgr->GiveBack((char*)event);
        }
    }
}