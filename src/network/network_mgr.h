#pragma once
#include <atomic>
#include <thread>
#include "src/tools/ringbuffer.h"
#include "event.h"
#include "network.h"

class INetwork;

// 网络类型
enum NetworkType
{
    Unknown = 0,
    NT_TCP,
    NT_MAX,
};

using Event2Main = RingBuffer<NetEventMain*, 1024>;
/*
* 定义网络主线程
*/
class NetworkMaster
{
public:
    /*
    * 构造
    */
    NetworkMaster();
    /*
    * 析构
    */
    virtual ~NetworkMaster();
    /*
    * 启动工作线程
    */
    virtual bool Start();
    /*
    * 驱动主线程内事件处理
    */
    virtual void Update();
    /*
    * 结束并等待工作线程结束,在主线程内调用
    */
    virtual void StopWait();
    /*
    * 通知工作线程关闭连接
    */
    void Close(NetworkType type, uint64_t conn_id);
    /*
    * 通知工作线程发送数据
    */
    void Send(NetworkType type, uint64_t conn_id, const char* data, uint32_t size);
    /*
    * 工作线程投递事件到主线程
    * @param event 事件
    */
   void NotifyMain(NetEventMain* event);
protected:
    /*
    * 主线程内处理接受新连接事件
    * @param conn_id 连接ID
    */
    virtual void OnAccepted(uint64_t conn_id){};
    /*
    * 主线程内处理主动连接事件
    * @param conn_id 连接ID
    */
    virtual void OnConnected(uint64_t conn_id){};
    /*
    * 主线程内处理主动连接事件
    * @param conn_id 连接ID
    */
    virtual void OnConnectedFailed(ENetErrCode err_code, int32_t err_no){};
    /*
    * 主线程内处理连接关闭事件
    * @param conn_id 连接ID
    */
    virtual void OnClose(uint64_t conn_id, ENetErrCode net_err, int32_t sys_err){};
    /*
    * 主线程内处理数据可读事件
    * @param conn_id 连接ID
    * @param data 内存指针
    * @param size 数据长度
    */
    virtual void OnReceived(uint64_t conn_id, const char* data, size_t size){};
public:
    /*
    * 通知工作线程建立一个监听器
    * @param ip 监听ip
    * @param port 监听端口
    * @param type 网络类型
    * @param send_buff_size 发送缓冲区大小
    * @param recv_buff_size 接收缓冲区大小
    */
    void Accept(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
    /*
    * 通知工作线程建立一个主动连接
    * @param ip 连接ip
    * @param port 连接端口
    * @param type 网络类型
    * @param send_buff_size 发送缓冲区大小
    * @param recv_buff_size 接收缓冲区大小
    */
    void Connect(const std::string& ip, uint16_t port, NetworkType type, int32_t send_buff_size = 0, int32_t recv_buff_size = 0);
    /*
    * 主线程投递事件到工作线程
    * @param event 事件
    * @param type 网络类型
    */
    void NotifyWorker(NetEventWorker* event, NetworkType type);

private:
    /*
    * 处理需要在主线程处理的事件
    */
    void DispatchMainEvent_();
    /*
    * 根据类型获取网络实例
    * @param type 网络类型
    */
    INetwork* GetNetwork_(NetworkType type);

private:
    Event2Main event2main_;     // 主线程网络事件队列
    std::atomic_bool stop_;     // 线程退出
    std::unique_ptr<std::thread> worker_ = nullptr;   // 工作线程
    using NetworkArray = std::array<std::unique_ptr<INetwork>, NT_MAX>;
    NetworkArray networks_;     // 网络实现 
};

