#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "network/net_imp/imp_network.h"
#include "network/net_imp/socket_pool.h"
#include "network/net_imp/tcp_socket.h"
#include "iocp_ctrl.h"

namespace ToolBox
{

    /*
    * 定义基于 TCP 和 IOCP 的网络
    */
    class TcpIocpNetwork : public ImpNetwork<TcpSocket>
    {
    public:
        /*
        * 构造
        */
        TcpIocpNetwork() = default;
        /*
        * 析构
        */
        virtual ~TcpIocpNetwork() = default;
        /*
        * 初始化
        */
        virtual bool Init(NetworkMaster* master, NetworkType network_type) override;
    };

};  // ToolBox

#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
