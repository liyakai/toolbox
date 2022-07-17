#pragma once

#if defined(__APPLE__)
#include "network/net_imp/net_imp_define.h"
#include "network/net_imp/base_ctrl.h"

namespace ToolBox
{

    /*
    * 定义 kqueue 管理类
    */
    class KqueueCtrl : public IOMultiplexingInterface
    {
    public:
        /*
        * 构造
        * @param max_events 最大事件数量
        */
        KqueueCtrl(uint32_t max_events);
        /*
        * 创建 Kqueue
        * @return 是否成功
        */
        bool CreateIOMultiplexing() override;
        /*
        * 销毁 Kqueue
        */
        void DestroyIOMultiplexing() override;
        /*
        * 处理事件
        */
        bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) override;
        /*
        * @brief kevent 获取结果
        * @param msec 等待毫秒数
        * @return 有读写通知的文件描述符的个数
        */
        int32_t Kevent(int msec);
        /*
        * 执行一次 epoll wait
        */
        bool RunOnce()override;
    private:
        uint32_t fd_num_ = 0;       // 最大文件描述符监视数
        int kqueue_fd_ = 0;         // kqueue 文件描述符
        struct kevent* events_;     // kevent返回的事件列表
    };

};  // ToolBox

#endif // defined(__APPLE__)