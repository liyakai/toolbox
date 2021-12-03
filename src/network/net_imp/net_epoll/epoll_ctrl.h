#pragma once

#ifdef __linux__

#include "src/tools/singleton.h"
#include <sys/epoll.h>
#include "epoll_define.h"
#include "src/network/net_imp/tcp_socket.h"

/*
* epoll 类
*/
class EpollCtrl
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    EpollCtrl(uint max_events);
    /*
    * 创建 epoll
    * @return 是否成功
    */
    bool CreateEpoll();
    /*
    * 销毁 epoll
    */
    void Destroy();
    /*
    * 添加事件
    * @param socket_fd 文件描述符
    * @param event  事件类型
    * @param ptr 透传指针
    * @return 是否成功
    */
    bool AddEvent(int socket_fd, int event, void *ptr);
    /*
    * 修改事件
    * @param socket_fd 文件描述符
    * @param event  事件类型
    * @param ptr 透传指针
    * @return 是否成功
    */
    bool ModEvent(int socket_fd, int event, void *ptr);
    /*
    * 删除事件
    * @param socket_fd 文件描述符
    */
    bool DelEvent(int socket_fd);
    /*
    * 处理事件
    */
    template<typename SocketType>
    bool OperEvent(SocketType &socket, EventOperType op_type, int32_t event_type)
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
    * 获取事件
    * @param index 事件序号
    * @return epoll_event*
    */
    epoll_event *GetEvent(int index);
    /*
    * 执行一次 epoll wait
    */
    template<typename SocketType>
    bool RunOnce()
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
            SocketType* socket = static_cast<SocketType*>(event.data.ptr);
            if (nullptr == socket) continue;
            if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);
            }
            else if (event.events & EPOLLIN)
            {
                socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
            }
            else if (event.events & EPOLLOUT)
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

#endif // __linux__