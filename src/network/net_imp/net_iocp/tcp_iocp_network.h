#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "src/network/net_imp/imp_network.h"
#include "src/network/net_imp/socket_pool.h"
#include "src/network/net_imp/tcp_socket.h"
#include "iocp_ctrl.h"

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
    virtual void Init(NetworkMaster* master, NetworkType network_type) override;
};

#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
