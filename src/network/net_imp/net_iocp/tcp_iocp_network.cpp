#include "tcp_iocp_network.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "src/network/net_imp/net_imp_define.h"

namespace ToolBox{

void TcpIocpNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    base_ctrl_ = new IocpCtrl();
    ImpNetwork<TcpSocket>::Init(master, network_type);
}

};  // ToolBox

#endif // __linux__
