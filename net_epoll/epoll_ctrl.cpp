#include "epoll_ctrl.h"
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

EpollCtrl::EpollCtrl(uint max_events)
    : max_events_(max_events)
{
}

bool EpollCtrl::CreateEpoll()
{
    epoll_fd_ = epoll_create(max_events_);
    if (epoll_fd_ < 0)
    {
        return false;
    }
    events_ = new epoll_event[max_events_];

    return true;
}
void EpollCtrl::Destroy()
{
    if (events_ != nullptr)
    {
        delete[] events_;
        events_ = nullptr;
    }
    if (-1 != epoll_fd_)
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

bool EpollCtrl::AddEvent(int socket_fd, int event, void *ptr)
{
    epoll_event evt;
    evt.data.ptr = ptr;
    evt.events = event;

    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd, &evt) == 0;
}

bool EpollCtrl::ModEvent(int socket_fd, int event, void *ptr)
{
    epoll_event evt;
    evt.events = event;
    evt.data.ptr = ptr;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_fd, &evt) == 0;
}

bool EpollCtrl::DelEvent(int socket_fd)
{
    epoll_event evt;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd, &evt) == 0;
}

bool EpollCtrl::OperEvent(EpollSocket &socket, EpollOperType op_type, SockEventType event_type)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    const SockEventType now_event = socket.GetEventType();
    event.data.ptr = &socket;
    event.events |= EPOLLET;
    if (event_type & SOCKET_EVENT_RECV)
    {
        if (now_event & SOCKET_EVENT_SEND) // 已经注册写事件
        {
            event.events |= EPOLLOUT;
        }
        if (EpollOperType::EPOLL_OPER_ADD == op_type)
        {
            event.events |= EPOLLIN;
        }
    }
    else if (event_type & SOCKET_EVENT_SEND)
    {
        if (now_event & SOCKET_EVENT_RECV) // 已经注册读事件
        {
            event.events |= EPOLLIN;
        }
        if (EpollOperType::EPOLL_OPER_ADD == op_type)
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

int EpollCtrl::EpollWait(int msec)
{
    return epoll_wait(epoll_fd_, events_, max_events_, msec);
}

epoll_event *EpollCtrl::GetEvent(int index)
{
    if (index < 0 || index >= int(max_events_))
    {
        return nullptr;
    }
    return &events_[index];
}
bool EpollCtrl::RunOnce()
{
    epoll_event evt;
    int count = EpollWait(EPOLL_WAIT_MSECONDS);
    if (count < 0)
    {
        return false;
    }
    for (int i = 0; i < count; i++)
    {
        epoll_event &event = events_[i];
        EpollSocket *socket = static_cast<EpollSocket *>(event.data.ptr);
        if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
        {
            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);
        }
        else if (event.events & EPOLLIN)
        {
        }
        else if (event.events & EPOLLOUT)
        {
        }
    }
    return true;
}
