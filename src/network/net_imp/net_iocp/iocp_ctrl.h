#pragma once

#include "network/net_imp/net_imp_define.h"
#include "network/net_imp/base_ctrl.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

namespace ToolBox
{

    /*
    * 定义管理 iocp 的类
    */
    class IocpCtrl : public IOMultiplexingInterface
    {
    public:
        /*
        * 构造
        * @param max_events 最大事件数量
        */
        IocpCtrl();
        /*
        * 创建 iocp
        * @return 是否成功
        */
        bool CreateIOMultiplexing();
        /*
        * 销毁 iocp
        */
        void DestroyIOMultiplexing();
        /*
        * 处理事件
        */
        bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) override;
        /*
        * @brief 建立 socket 与 iocp 的关联
        */
        bool AssociateSocketToIocp(BaseSocket& socket)
        {
            // 建立 socket 与 iocp 的关联
            auto socket_id = socket.GetSocketID();
            HANDLE iocp = CreateIoCompletionPort((HANDLE)socket_id, iocp_fd_, (ULONG_PTR)&socket, 0);
            if (nullptr == iocp)
            {
                DWORD last_error = GetLastError();
                // MSDN:  If WSAGetLastError returns ERROR_IO_PENDING,
                // then the operation was successfully initiated and is still in progress.
                if (ERROR_IO_PENDING != last_error)
                {
                    socket.OnErrored(ENetErrCode::NET_SYS_ERROR, last_error);
                    return false;
                }

            }
            return true;
        }
        /*
        * @brief 处理接收消息
        */
        bool OnRecv(BaseSocket& socket);
        /*
        * @brief  处理发送消息
        */
        bool OnSend(BaseSocket& socket);
        /*
        * 执行一次 iocp
        */
        bool RunOnce() override;
    private:
        HANDLE iocp_fd_;            // iocp 文件描述符
    };

};  // ToolBox

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)