#pragma once

#if defined(__APPLE__)

#include "src/network/network.h"
#include "src/network/net_imp/socket_pool.h"
#include "src/network/net_imp/tcp_socket.h"
#include "kqueue_ctrl.h"

/*
* 定义基于 TCP 和 Kqueue 的网络
*/
class TcpKqueueNetwork : public INetwork
{
public:
    /*
    * 构造
    */
    TcpKqueueNetwork();
    /*
    * 析构
    */
    virtual ~TcpKqueueNetwork();
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
    KqueueCtrl& GetIocpCtrl() { return kqueue_ctrl_; }
   
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
    KqueueCtrl kqueue_ctrl_;            // kqueue 管理器
    SocketPool<TcpSocket> sock_mgr_;    // socket 池
};

#endif // defined(__APPLE__)