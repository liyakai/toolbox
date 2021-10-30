#pragma once

#include "src/network/network.h"
#include "epoll_ctrl.h"
#include "epoll_socket_pool.h"

/*
* 定义 Epoll网络
*/
class EpollNetwork : public INetwork
{
public:
    /*
    * 构造
    */
    EpollNetwork();
    /*
    * 析构
    */
    virtual ~EpollNetwork();
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
    /*
    * 工作线程内闭网络连接
    */
    virtual void OnClose(uint64_t connect_id) override;
    /*
    * 工作线程内工作线程内发送
    */
    virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size) override;

private:
    NetworkMaster *network_master_;     // 主线程网络管理器
    EpollCtrl epoll_ctrl_;              // epoll控制器
    EpollSocketPool sock_mgr_;          // socket池
};
