#include "tcp_iocp_network.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "src/network/net_imp/net_imp_define.h"

namespace ToolBox
{

    bool TcpIocpNetwork::Init(NetworkMaster* master, NetworkType network_type)
    {
        base_ctrl_ = new IocpCtrl();
        if (!ImpNetwork<TcpSocket>::Init(master, network_type))
        {
            NetworkLogError("[Network] Init TcpIocpNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

};  // ToolBox

#endif // __linux__
