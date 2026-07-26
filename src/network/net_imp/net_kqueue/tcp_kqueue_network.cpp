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
        if (!base_ctrl_->DelEvent(socket_id))
        {
            NetworkLogWarn("[Network] remove kqueue socket failed. fd:%d error:%d",
                           socket_id, base_ctrl_->GetLastSystemError());
        }
    }

};  // ToolBox

#endif // defined(__APPLE__)