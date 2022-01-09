#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace ToolBox{

enum class ENetErrCode;

enum EventID
{
    EID_NONE = 0,
    EID_MainToWorkerNewAccepter,
    EID_MainToWorkerNewConnecter,
    EID_MainToWorkerClose,
    EID_MainToWorkerSend,
    EID_WorkerToMainBinded,
    EID_WorkerToMainBindFailed,
    EID_WorkerToMainConnected,
    EID_WorkerToMainConnectFailed,
    EID_WorkerToMainErrored,
    EID_WorkerToMainAccepted,
    EID_WorkerToMainClose,
    EID_WorkerToMainRecv,

    EID_MAX,
};

/*
* 定义事件
*/
class Event
{
public:
    /*
    * 构造
    * @param id 事件ID
    */
    Event(EventID id);
    /*
    * 析构
    */
    virtual ~Event();
    /*
    * 获取事件 ID
    */
    EventID GetID();
private:
    EventID id_ = EID_NONE;   // 事件ID
};

/*
* 定义工作线程内处理的事件
*/
class NetEventWorker : public Event
{
public:
    /*
    * 构造
    * @param type 事件类型
    */
    NetEventWorker(EventID event_id);
    /*
    * 析构
    */
    virtual ~NetEventWorker();
    /*
    * 设置 IP 
    */
    void SetIP(const std::string& ip);
    /*
    * 获取 IP
    */
    std::string GetIP() const;
    /*
    * 设置 Port 
    */
    void SetAddressPort(const uint16_t port);
    /*
    * 获取 Port
    */
    uint16_t GetPort() const;
    /*
    * 设置 buff 大小
    */
    void SetBuffSize(int32_t send_size, int32_t recv_size);
    /*
    * 获取发送 buff 大小
    */
    int32_t GetSendBuffSize();
    /*
    * 获取 接收 buff 大小
    */
    int32_t GetRecvBuffSize();
    /*
    * 设置 连接ID
    */
    void SetConnectID(const uint64_t conn_id);
    /*
    * 获取 连接ID
    */
    uint64_t GetConnectID() const;
    /*
    * 设置 数据
    */
    void SetData(const char* data, uint32_t size);
    /*
    * 获取 数据
    */
    char* GetData() const;
    /*
    * 获取数据大小
    */
    uint32_t GetDataSize() const;


private:
    union NetReq
    {
        struct SendReq
        {
            uint64_t connect_id_;
            char* data_;
            int32_t size_;
        } stream_;
        struct Address
        {
            std::string* ip_;
            uint16_t port_;
            int32_t send_buff_size;
            int32_t recv_buff_size;
        } address_;
        NetReq(){}
        ~NetReq(){};
    } net_req_;
};

/*
* 定义主线程内处理的网络事件
*/
class NetEventMain : public Event
{
public:
    /*
    * 构造
    */
    NetEventMain(EventID event_id);
    /*
    * 析构
    */
    virtual ~NetEventMain();
public:
    union NetEvt
    {
        struct ConnectSucessed
        {
            uint64_t connect_id_;
        } connect_sucessed_;
        struct ConnectFailedEvt
        {
            ENetErrCode net_err_code;
            int32_t sys_err_code;
        } connect_failed_;
        struct Bind
        {
            uint64_t connect_id_;
        } bind_;
        struct Accept
        {
            uint64_t connect_id_;
        } accept_;
        struct Recv
        {
            uint64_t connect_id_;
            const char* data_;
            uint32_t size_;
        } recv_;
        struct Error 
        {
            uint64_t connect_id_;
            ENetErrCode net_err_code;
            int32_t sys_err_code;
        } error_;
        struct Close
        {
            uint64_t connect_id_;
            ENetErrCode net_err_;
            int32_t sys_err_;
        } close_;
    } net_evt_;
    
};

using EventHandle = std::function<void(Event* event)>;
/*
* 可以处理事件的对象
*/
class EventBasedObject
{
public:
    /*
    * 构造
    */
    EventBasedObject();
    /*
    * 析构
    */
    virtual ~EventBasedObject();
    /*
    * 处理事件
    * @param event 事件指针
    */
    void HandleEvent(Event* event);
    /*
    * 注册事件函数
    * @param event_id 事件ID
    * @param func 事件函数
    */
    void RegistereventHandler(EventID event_id, EventHandle func);
    /*
    * 取消事件注册
    * @param event_id
    */
   void UnregisterEventHandler(EventID event_id);
private:
    using EventFuncMap = std::unordered_map<std::uint32_t, EventHandle>;
    EventFuncMap event_func_map_; // 事件处理函数表
};

};  // ToolBox