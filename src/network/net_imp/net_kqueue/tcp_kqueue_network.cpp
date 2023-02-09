#include "tcp_kqueue_network.h"
#include "kqueue_define.h"

#if defined(__APPLE__)

namespace ToolBox
{

    bool TcpKqueueNetwork::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        base_ctrl_ = new KqueueCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<TcpSocket>::Init(master, network_type, net_thread_index))
        {
            NetworkLogError("[Network] Init TcpKqueueNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
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

};  // ToolBox

#endif // defined(__APPLE__)