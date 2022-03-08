#include "tcp_io_uring_network.h"

#ifdef __linux__

#include "io_uring_ctrl.h"
#include "src/network/net_imp/socket_pool.h"

namespace ToolBox{

void TcpIOUringNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    base_ctrl_ = new IOUringCtrl(MAX_SOCKET_COUNT);
    ImpNetwork<TcpSocket>::Init(master, network_type);
}


};  // ToolBox

#endif // __linux__





