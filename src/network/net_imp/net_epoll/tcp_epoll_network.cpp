#include "tcp_epoll_network.h"

#ifdef __linux__

#include "network/net_imp/net_imp_define.h"
#include "epoll_ctrl.h"

namespace ToolBox
{

    bool TcpEpollNetwork::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        base_ctrl_ = new EpollCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<TcpSocket>::Init(master, network_type, net_thread_index))
        {
            NetworkLogError("[Network] Init TcpEpollNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

};  // ToolBox

#endif // __linux__
