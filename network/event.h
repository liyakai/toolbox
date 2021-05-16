#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <functional>

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
    Event(uint32_t id);
    /*
    * 析构
    */
    ~Event();
    /*
    * 获取事件 ID
    */
    uint32_t GetID();
private:
    uint32_t id_ = 0;   // 事件ID
};

enum NetEventWorkerType
{
    MainToWorkerNewAccepter = 1,
    MainToWorkerNewConnecter,
    MainToWorkerClose,
    MainToWorkerSend,

    MainToWorkerMax,
};

/*
* 定义工作线程内处理的事件
*/
class NetEventWorker
{
public:
    /*
    * 构造
    * @param type 事件类型
    */
    NetEventWorker(NetEventWorkerType type);
    /*
    * 析构
    */
    virtual ~NetEventWorker();
    /*
    * 获取事件类型
    */
    NetEventWorkerType GetType(){ return type_; };
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
    void SetPort(const uint16_t port);
    /*
    * 获取 Port
    */
    uint16_t GetPort() const;
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
    const char* GetData() const;
    /*
    * 获取数据大小
    */
    uint32_t GetDataSize() const;


private:
    union Detail
    {
        struct Stream
        {
            uint64_t connect_id_;
            char* data_;
            uint32_t size_;
        } stream_;
        struct Address
        {
            std::string ip_;
            uint16_t port_;
        } address_;
        Detail(){}
        ~Detail(){};
    } detail_;
    NetEventWorkerType type_;   // 事件类型
};

enum NetEvenMainType
{
    WorkerToMainBinded = static_cast<int>(NetEventWorkerType::MainToWorkerMax) + 1,
    WorkerToMainBindFailed,
    WorkerToMainConnected,
    WorkerToMainConnectFailed,
    WorkerToMainAccepted,
    WorkerToMainClose,
    WorkerToMainRecv,

    MainToMainMax,
};

/*
* 定义主线程内处理的网络事件
*/
class NetEventMain
{
public:
    /*
    * 构造
    */
    NetEventMain(NetEvenMainType type);
    /*
    * 析构
    */
    virtual ~NetEventMain();
    /*
    * 获取事件类型
    */
    NetEvenMainType GetType(){ return type_; };
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
    const char* GetData() const;
    /*
    * 获取数据大小
    */
    uint32_t GetDataSize() const;
private:
    NetEvenMainType type_;
    struct Stream
    {
        uint64_t connect_id_;
        char* data_;
        uint32_t size_;
    } stream_;
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
    void RegistereventHandler(uint32_t event_id, EventHandle func);
    /*
    * 取消事件注册
    * @param event_id
    */
   void UnregisterEventHandler(uint32_t event_id);
private:
    using EventFuncMap = std::unordered_map<std::uint32_t, EventHandle>;
    EventFuncMap event_func_map_; // 事件处理函数表
};
