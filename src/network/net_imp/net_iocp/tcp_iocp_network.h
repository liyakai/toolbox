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
    TcpIocpNetwork();
    /*
    * 析构
    */
    virtual ~TcpIocpNetwork();
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master, NetworkType network_type) override;
    /*
    * 逆初始化网络
    */
    virtual void UnInit() override;
    /*
    * 执行一次网络循环
    */
    virtual void Update() override;
public:
    IocpCtrl& GetIocpCtrl() { return iocp_ctrl_; }
   
protected:
    /*
    * 工作线程内建立监听器
    */
    virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
    /*
    * 工作线程内建立连接器
    */
    virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
private:
    IocpCtrl iocp_ctrl_;                // iocp 控制器
};

#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
