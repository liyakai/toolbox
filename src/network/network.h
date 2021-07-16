#pragma once

#include "event.h"
#include "src/tools/ringbuffer.h"
#include "src/tools/memory_pool.h"
#include "network_mgr.h"

class NetworkMaster;

enum class ENetErrCode
{
    NET_NO_ERROR = 0,
    NET_SYS_ERROR,            // 系统错误，同时会返回errno
    NET_INVALID_PACKET_SIZE,  // 错误的包长
    NET_CONNECT_FAILED,       // 连接出错
    NET_LISTEN_FAILED,        // 监听出错
    NET_ACCEPT_FAILED,        // accept 出错
    NET_SEND_FAILED,          // 发包出错
    NET_RECV_FAILED,          // 收包出错
    NET_ALLOC_FAILED,         // 申请内存出错，在socket对象申请上
    NET_SEND_BUFF_OVERFLOW,   // 发送缓冲区满
    NET_RECV_BUFF_OVERFLOW,   // 接收缓冲区满
    NET_ENCODE_BUFF_OVERFLOW, // 打包缓冲区满
    NET_DECODE_BUFF_OVERFLOW, // 解包缓冲区满

    NET_SEND_PIPE_OVERFLOW, // 发送ringbuffer满
    NET_RECV_PIPE_OVERFLOW, // 接收ringbuffer满
};

/// 事件队列
using Event2Worker = RingBuffer<NetEventWorker *, 1024>;
/// 事件处理函数
using EventHandler = std::function<void(Event *event)>;
/*
* Worker 网络基类
*/
class INetwork : public EventBasedObject
{
public:
    /*
    * 构造
    */
    INetwork();
    /*
    * 析构
    */
    virtual ~INetwork();
    /*
    * 初始化网络
    */
    virtual void Init(NetworkMaster *master);
    /*
    * 逆初始化网络
    */
    virtual void UnInit();
    /*
    * 运行一次网络循环 派生类要调用基类的 Update()
    */
    virtual void Update();
    /*
    * 加入事件
    */
    void PushEvent(NetEventWorker *event);

protected:
    /*
    * 主线程通知,工作线程内建立监听器
    */
    virtual uint64_t OnNewAccepter(const std::string &ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
    /*
    * 主线程通知,工作线程内建立连接器
    */
    virtual uint64_t OnNewConnecter(const std::string &ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) = 0;
    /*
    * 主线程通知,关工作线程内闭网络连接
    */
    virtual void OnClose(uint64_t connect_id) = 0;
    /*
    * 主线程通知,工作线程内工作线程内发送
    */
    virtual void OnSend(uint64_t connect_id, const char *data, uint32_t size) = 0;

public:
    /*
    * 工作线程内接收到新连接,通知主线程
    */
    void OnAccepted(uint64_t connect_id);
    /*
    * 工作线程内连接到远端成功,通知主线程
    */
    void OnConnected(uint64_t connect_id);
    /*
    * 工作线程内连接到远端失败,通知主线程
    */
    void OnConnectedFailed(ENetErrCode err_code, int32_t err_no);
    /*
    * 工作线程内关闭网络连接,通知主线程
    */
    void OnClosed(uint64_t connect_id, int32_t net_err, int32_t sys_err);
    /*
    * 工作线程内接收到数据,通知主线程
    */
    void OnReceived(uint64_t connect_id, const char *data, uint32_t size);

private:
    /*
    * 通知工作线程建立监听器
    */
    void OnMainToWorkerNewAccepter_(Event *event);
    /*
    * 通知工作线程建立连接器
    */
    void OnMainToWorkerNewConnecter_(Event *event);
    /*
    * 通知工作线程关闭网络连接
    */
    void OnMainToWorkerClose_(Event *event);
    /*
    * 通知工作线程发送消息
    */
    void OnMainToWorkerSend_(Event *event);
    /*
    * 处理需要在工作线程中处理的事件
    */
    void HandleEvents_();

private:
    Event2Worker event2worker_;
    NetworkMaster *master_;
};
