#pragma once
#ifdef LINUX_IO_URING

#include "src/network/net_imp/imp_network.h"

namespace ToolBox{

/*
* 定义基于 TCP 和 io_uring 的网络
*/
class TcpIOUringNetwork : public ImpNetwork<TcpSocket>
{
public:
    /*
    * 构造
    */
    TcpIOUringNetwork() = default;
    /*
    * 析构
    */
    virtual ~TcpIOUringNetwork() = default;
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master, NetworkType network_type) override;
};

};  // ToolBox

#endif // LINUX_IO_URING













