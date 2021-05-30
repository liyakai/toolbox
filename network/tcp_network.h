#pragma once

#include "network.h"
#include "net_epoll/epoll_ctrl.h"

/*
* 定义 Tcp网络
*/
class TcpNetwork : public INetwork
{
public:
    /*
    * 构造
    */
    TcpNetwork();
    /*
    * 析构
    */
    virtual ~TcpNetwork();
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master) override;
    /*
    * 逆初始化网络
    */
    virtual void UnInit() override;
    /*
    * 执行一次网络循环
    */
    virtual void Update() override;
   
protected:
    /*
    * 工作线程内建立监听器
    */
    virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port) override;
    /*
    * 工作线程内建立连接器
    */
    virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port) override;
    /*
    * 关工作线程内闭网络连接
    */
    virtual void OnClose(uint64_t connect_id) override;
    /*
    * 工作线程内工作线程内发送
    */
    virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size) override;

private:
    NetworkMaster *network_master_;
    EpollCtrl epoll_ctrl_;
};
