#pragma once

#if defined(__APPLE__)

#include "src/network/net_imp/imp_network.h"
#include "src/network/net_imp/socket_pool.h"
#include "src/network/net_imp/tcp_socket.h"
#include "kqueue_ctrl.h"

/*
* 定义基于 TCP 和 Kqueue 的网络
*/
class TcpKqueueNetwork  : public ImpNetwork<TcpSocket>
{
public:
    /*
    * 构造
    */
    TcpKqueueNetwork() = default;
    /*
    * 析构
    */
    virtual ~TcpKqueueNetwork() = default;
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master, NetworkType network_type) override;
    /*
    * @brief 在io多路复用中关闭监听socket
    * @param socket 的文件描述符
    */
    virtual void CloseListenInMultiplexing(int32_t socket_id) override;
};

#endif // defined(__APPLE__)