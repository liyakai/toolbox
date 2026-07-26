#pragma once
#include "net_imp_define.h"
#include "base_socket.h"
#include <cstdint>
#include <functional>

namespace ToolBox
{

    using SockDispatchResolver = std::function<BaseSocket*(uint16_t pool_index, uint32_t generation)>;

    /*
    * @brief IOMultiplexingInterface 是io多路复用的接口.
    */
    class IOMultiplexingInterface
    {
    public:
        /*
        * 基类虚析构
        */
        virtual ~IOMultiplexingInterface() {};
        /*
        * 创建IO多路复用
        * @return 是否成功
        */
        virtual bool CreateIOMultiplexing() = 0;
        /*
        * 销毁IO多路复用
        */
        virtual void DestroyIOMultiplexing() = 0;
        /*
        * 在 IO多路复用上 添加/删除 socket
        */
        virtual bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) = 0;
        /*
        * 在IO多路复用删除 socket
        */
        virtual bool DelEvent(int socket_fd)
        {
            return true;
        };
        /*
        * 执行一次. e.g. epollwait;GetQueuedCompletionStatus;kevent.
        */
        virtual bool RunOnce(std::time_t time_stamp) = 0;
        /*
        * 最近一次控制操作的系统错误码
        */
        int32_t GetLastSystemError() const
        {
            return last_system_error_;
        }
        /*
        * 设置派发令牌解析(由 ImpNetwork 注入池查询)
        */
        void SetSockDispatchResolver(SockDispatchResolver resolver)
        {
            sock_dispatch_resolver_ = std::move(resolver);
        }
    protected:
        void SetLastSystemError(int32_t error)
        {
            last_system_error_ = error;
        }
        /*
        * 按令牌解析存活 socket;过期/已 Free 返回 nullptr
        */
        BaseSocket* ResolveSockDispatchToken(uint64_t token) const
        {
            if (!sock_dispatch_resolver_)
            {
                return nullptr;
            }
            return sock_dispatch_resolver_(UnpackSockDispatchIndex(token),
                                           UnpackSockDispatchGeneration(token));
        }

    private:
        int32_t last_system_error_ = 0;
        SockDispatchResolver sock_dispatch_resolver_;
    };

};  // ToolBox
