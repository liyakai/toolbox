#pragma once

#include "src/network/network.h"
#include <unordered_map>
#include "epoll_ctrl.h"
#include "socket_pool.h"

class UdpSocket;
class UdpAddress;
/*
* 定义基于 UDP 和 Epoll 的网络
*/
class UdpEpollNetwork : public INetwork
{
public:
    /*
    * 构造
    */
    UdpEpollNetwork();
    /*
    * 析构
    */
    virtual ~UdpEpollNetwork();
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
    /*
    * @brief UdpAddress 是否存在
    */
    bool IsUdpAddressExist(const UdpAddress& udp_address);
   
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
    virtual void OnSend(uint64_t connect_id, const char* data, std::size_t size) override;

private:
    NetworkMaster *network_master_;     // 主线程网络管理器
    EpollCtrl epoll_ctrl_;              // epoll控制器
    SocketPool<UdpSocket> sock_mgr_;    // socket池
    std::unordered_map<uint64_t, uint32_t> address_to_connect_;      // 地址转换的ID 到 SocketPool管理的连接ID的映射
};
