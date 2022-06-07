#include "tcp_epoll_network.h"

#ifdef __linux__

#include "src/network/net_imp/net_imp_define.h"
#include "epoll_ctrl.h"

namespace ToolBox
{

    bool TcpEpollNetwork::Init(NetworkMaster* master, NetworkType network_type)
    {
        base_ctrl_ = new EpollCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<TcpSocket>::Init(master, network_type))
        {
            NetworkLogError("[Network] Init TcpEpollNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

};  // ToolBox

#endif // __linux__
