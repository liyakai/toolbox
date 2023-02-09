#include "tcp_io_uring_network.h"

#ifdef LINUX_IO_URING

#include "io_uring_ctrl.h"
#include "network/net_imp/socket_pool.h"
#include "network/net_imp/net_imp_define.h"

namespace ToolBox
{

    bool TcpIOUringNetwork::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        base_ctrl_ = new IOUringCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<TcpSocket>::Init(master, network_type, net_thread_index))
        {
            NetworkLogError("[Network] Init TcpIOUringNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }


};  // ToolBox

#endif // LINUX_IO_URING





