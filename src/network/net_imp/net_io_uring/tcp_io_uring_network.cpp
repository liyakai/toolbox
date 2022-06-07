#include "tcp_io_uring_network.h"

#ifdef LINUX_IO_URING

#include "io_uring_ctrl.h"
#include "src/network/net_imp/socket_pool.h"
#include "src/network/net_imp/net_imp_define.h"

namespace ToolBox
{

    bool TcpIOUringNetwork::Init(NetworkMaster* master, NetworkType network_type)
    {
        base_ctrl_ = new IOUringCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<TcpSocket>::Init(master, network_type))
        {
            NetworkLogError("[Network] Init TcpIOUringNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }


};  // ToolBox

#endif // LINUX_IO_URING





