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

