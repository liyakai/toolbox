#pragma once

#ifdef __linux__

#include "network/net_imp/imp_network.h"


namespace ToolBox
{

    class TcpSocket;

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
        virtual bool Init(NetworkMaster* master, NetworkType network_type) override;
    };

};  // ToolBox

#endif // __linux__
