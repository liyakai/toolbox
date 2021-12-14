#pragma once

#ifdef __linux__

#include "src/network/net_imp/imp_network.h"
#include "epoll_ctrl.h"
#include "src/network/net_imp/socket_pool.h"

/*
* 定义基于 TCP 和 Epoll 的网络
*/
class TcpEpollNetwork : public ImpNetwork<TcpSocket>
{
public:
    /*
    * 构造
    */
    TcpEpollNetwork();
    /*
    * 析构
    */
    virtual ~TcpEpollNetwork();
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
    /*
    * @brief 在io多路复用中关闭监听socket
    * @param socket 的文件描述符
    */
    virtual void CloseListenInMultiplexing(int32_t socket_id) override;
public:
    EpollCtrl& GetEpollCtrl(){ return epoll_ctrl_;}
   
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
    EpollCtrl epoll_ctrl_;              // epoll控制器
};

#endif // __linux__
