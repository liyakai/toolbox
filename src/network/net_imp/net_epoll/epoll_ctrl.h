#pragma once

#ifdef __linux__
#include "../base_ctrl.h"
#include "tools/singleton.h"
#include <cerrno>
#include <sys/epoll.h>

namespace ToolBox
{

    /*
    * epoll 类
    */
    class EpollCtrl : public IOMultiplexingInterface
    {
    public:
        /*
        * 构造
        * @param max_events 最大事件数量
        */
        EpollCtrl(uint32_t max_events);
        /*
        * 创建 epoll
        * @return 是否成功
        */
        bool CreateIOMultiplexing() override;
        /*
        * 销毁 epoll
        */
        void DestroyIOMultiplexing() override;
        /*
        * 删除事件
        * @param socket_fd 文件描述符
        */
        bool DelEvent(int socket_fd) override;
        /*
        * 处理事件
        */
        bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) override
        {
            SetLastSystemError(0);
            epoll_event event;
            memset(&event, 0, sizeof(event));
            const auto now_event = socket.GetEventType();
            event.data.ptr = &socket;
            event.events |= EPOLLET;
            if (event_type & SOCKET_EVENT_RECV)
            {
                if (now_event & SOCKET_EVENT_SEND) // 已经注册写事件
                {
                    event.events |= EPOLLOUT;
                }
                if (EventOperType::EVENT_OPER_ADD == op_type)
                {
                    event.events |= EPOLLIN;
                }
            }
            if (event_type & SOCKET_EVENT_SEND)
            {
                if (now_event & SOCKET_EVENT_RECV) // 已经注册读事件
                {
                    event.events |= EPOLLIN;
                }
                if (EventOperType::EVENT_OPER_ADD == op_type)
                {
                    event.events |= EPOLLOUT;
                }
            }
            int32_t result = 0;
            if (socket.IsCtrlAdd())
            {
                result = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket.GetSocketID(), &event);
            }
            else
            {
                result = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket.GetSocketID(), &event);
                // 仅 ADD 成功才置标记,否则下次会误走 MOD
                if (0 == result)
                {
                    socket.SetCtrlAdd(true);
                }
            }
            if (0 != result)
            {
                SetLastSystemError(errno);
                NetworkLogError("[Network] epoll_ctl failed. fd:%d, is_ctrl_add:%d, errno:%d",
                                socket.GetSocketID(), socket.IsCtrlAdd(), errno);
            }
            return 0 == result;
        }
        /*
        * epoll_wait
        * @param msec 等待毫秒数
        * @return 有读写通知的文件描述符的个数
        */
        int EpollWait(int msec);
        /*
        * 执行一次 epoll wait
        */
        bool RunOnce(std::time_t time_stamp) override
        {
            int32_t count = EpollWait(EPOLL_WAIT_MSECONDS);
            if (count < 0)
            {
                // EINTR 为信号打断,非错误,本轮空转下次重试
                if (EINTR == errno)
                {
                    return true;
                }
                SetLastSystemError(errno);
                return false;
            }
            for (int32_t i = 0; i < count; i++)
            {
                epoll_event& event = events_[i];
                BaseSocket* socket = static_cast<BaseSocket*>(event.data.ptr);
                if (nullptr == socket)
                {
                    continue;
                }
                if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
                {
                    socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                    continue;
                }
                if (event.events & EPOLLIN)
                {
                    socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
                }
                if (event.events & EPOLLOUT)
                {
                    socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
                }
            }
            return true;
        }
    private:
        uint32_t max_events_ = 0; // 最大事件数
        int epoll_fd_ = -1;            // epoll 文件描述符
        epoll_event* events_ = nullptr;     // epoll 事件数组
    };

#define EpollCtrlMgr Singleton<ToolBox::EpollCtrl>::Instance(10240)

};  // ToolBox

#endif // __linux__