#pragma once

#ifdef __linux__

#include "src/network/net_imp/imp_network.h"
#include "src/network/net_imp/tcp_socket.h"


namespace ToolBox{

/*
* 定义基于 TCP 和 Epoll 的网络
*/
class TcpEpollNetwork : public ImpNetwork<TcpSocket>
{
public:
    /*
    * 构造
    */
    TcpEpollNetwork() = default;
    /*
    * 析构
    */
    virtual ~TcpEpollNetwork() = default;
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master, NetworkType network_type) override;
};

};  // ToolBox

#endif // __linux__
