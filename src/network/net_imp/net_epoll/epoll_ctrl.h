#pragma once

#ifdef __linux__

#include "src/tools/singleton.h"
#include <sys/epoll.h>
#include "src/network/net_imp/net_imp_define.h"
#include "src/network/net_imp/tcp_socket.h"
#include "src/network/net_imp/base_ctrl.h"

namespace ToolBox{

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
    bool DelEvent(int socket_fd);
    /*
    * 处理事件
    */
    bool OperEvent(BaseSocket &socket, EventOperType op_type, int32_t event_type) override
    {
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
        if (socket.IsCtrlAdd())
        {
            epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket.GetSocketID(), &event);
        }
        else
        {
            socket.SetCtrlAdd(true);
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket.GetSocketID(), &event);
        }

        return true;
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
    bool RunOnce() override
    {
        epoll_event evt;
        time_t time_stamp = time(0);    // 时间戳
        int32_t count = EpollWait(EPOLL_WAIT_MSECONDS);
        if (count < 0)
        {
            return false;
        }
        for (int32_t i = 0; i < count; i++)
        {
            epoll_event& event = events_[i];
            BaseSocket* socket = static_cast<BaseSocket*>(event.data.ptr);
            if (nullptr == socket) continue;
            if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);
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
    int epoll_fd_;            // epoll 文件描述符
    epoll_event *events_;     // epoll 事件数组
};

#define EpollCtrlMgr Singleton<EpollCtrl>::Instance(10240)

};  // ToolBox

#endif // __linux__