#pragma once
#include <cstdint>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <functional>
#include "network_def_internal.h"

namespace ToolBox
{

    enum class ENetErrCode;

    enum EventID
    {
        EID_NONE = 0,
        EID_MainToWorkerNewAccepter,
        EID_MainToWorkerJoinIOMultiplexing,
        EID_MainToWorkerNewConnecter,
        EID_MainToWorkerClose,
        EID_MainToWorkerSend,
        EID_MainToWorkerSetSimulateNagle,
        EID_WorkerToMainBinded,
        EID_WorkerToMainBindFailed,
        EID_WorkerToMainConnected,
        EID_WorkerToMainConnectFailed,
        EID_WorkerToMainErrored,
        EID_WorkerToMainAcceptting,
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
        /*
        * 设置新连接的文件描述符
        */
        void SetFd(int32_t fd);
        /*
        * 获取新连接的文件描述符
        */
        int32_t GetFd() const;
        /*
        * 设置网络类型
        */
        void SetNetworkType(NetworkType network_type);
        /*
        * 获取网络类型
        */
        NetworkType GetNetworkType() const;
        /*
        * 设置特性参数
        */
        void SetFeatureParam(int32_t param1, int32_t param2);
        /*
        * 返回特性参数
        */
        std::tuple<int32_t, int32_t> GetFeatureParam();


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
                char ip_[16];
                uint16_t port_;
                int32_t send_buff_size;
                int32_t recv_buff_size;
                int32_t fd_;
            } address_;
            struct NetFeature
            {
                int32_t param1_ = 0;
                int32_t param2_ = 0;
            } net_feature_;
            NetReq() {}
            ~NetReq() {};
        } net_req_;
        NetworkType network_type_ = NT_UNKNOWN;
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
        /*
        * 设置 Bind IP
        */
        void SetBindIP(const std::string& ip);
        /*
        * 获取 Bind IP
        */
        std::string GetBindIP() const;
        /*
        * 设置 Acceptting IP
        */
        void SetAccepttingIP(const std::string& ip);
        /*
        * 获取 Acceptting IP
        */
        std::string GetAccepttingIP() const;

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
                char ip_[16];
                uint16_t port_;
            } bind_;
            struct Acceptting
            {
                int32_t fd_;
                char ip_[16];
                uint16_t port_;
                int32_t send_buff_size_;
                int32_t recv_buff_size_;
            } acceptting_;
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
        NetworkType network_type_ = NT_UNKNOWN;
    };

    using EventHandle = std::function<void(Event* event)>;
    /*
    * 事件分发器
    */
    class EventDispatcher
    {
    public:
        /*
        * 构造
        */
        EventDispatcher();
        /*
        * 析构
        */
        virtual ~EventDispatcher();
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
        using EventFuncArray = std::array<EventHandle, EID_MAX>;
        EventFuncArray event_func_array_; // 事件处理函数表
    };

};  // ToolBox