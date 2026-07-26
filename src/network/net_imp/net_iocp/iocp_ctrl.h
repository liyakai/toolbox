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
        * 处理事件.ADD:先 Associate 再投递 AcceptEx/WSARecv/WSASend
        */
        bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) override;
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
        bool RunOnce(std::time_t time_stamp) override;
    private:
        /*
        * 建立 socket 与 iocp 的关联(已关联时可重复调)
        */
        bool AssociateSocketToIocp(BaseSocket& socket);
        HANDLE iocp_fd_ = nullptr;  // iocp 文件描述符
    };

};  // ToolBox

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)