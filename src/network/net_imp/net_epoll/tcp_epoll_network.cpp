#include "tcp_epoll_network.h"

#ifdef __linux__

#include "epoll_define.h"
#include "epoll_ctrl.h"

namespace ToolBox{

void TcpEpollNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    base_ctrl_ = new EpollCtrl(MAX_SOCKET_COUNT);
    ImpNetwork<TcpSocket>::Init(master, network_type);
}

};  // ToolBox

#endif // __linux__
