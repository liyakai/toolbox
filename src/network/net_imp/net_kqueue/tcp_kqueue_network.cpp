#include "tcp_kqueue_network.h"
#include "kqueue_define.h"

#if defined(__APPLE__)

void TcpKqueueNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    base_ctrl_ = new KqueueCtrl(MAX_SOCKET_COUNT);
    ImpNetwork<TcpSocket>::Init(master, network_type);
}

void TcpKqueueNetwork::CloseListenInMultiplexing(int32_t socket_id)
{
    auto* socket = sock_mgr_.GetSocket(socket_id);
    if (nullptr != socket)
    {
        socket->SetSockEventType(0);
        base_ctrl_->OperEvent(*socket, EventOperType::EVENT_OPER_RDC, socket->GetEventType());
    }
}

#endif // defined(__APPLE__)