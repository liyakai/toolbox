#include "kqueue_ctrl.h"

#if defined(__APPLE__)


KqueueCtrl::KqueueCtrl(uint max_events)
    : max_events_(max_events)
{
}

bool KqueueCtrl::CreateEpoll()
{
    epoll_fd_ = epoll_create(max_events_);
    if (epoll_fd_ < 0)
    {
        return false;
    }
    events_ = new epoll_event[max_events_];

    return true;
}
void KqueueCtrl::Destroy()
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

bool KqueueCtrl::AddEvent(int socket_fd, int event, void *ptr)
{
    epoll_event evt;
    evt.data.ptr = ptr;
    evt.events = event;

    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd, &evt) == 0;
}

bool KqueueCtrl::ModEvent(int socket_fd, int event, void *ptr)
{
    epoll_event evt;
    evt.events = event;
    evt.data.ptr = ptr;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_fd, &evt) == 0;
}

bool KqueueCtrl::DelEvent(int socket_fd)
{
    epoll_event evt;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd, &evt) == 0;
}

int KqueueCtrl::EpollWait(int msec)
{
    return epoll_wait(epoll_fd_, events_, max_events_, msec);
}

epoll_event *KqueueCtrl::GetEvent(int index)
{
    if (index < 0 || index >= int(max_events_))
    {
        return nullptr;
    }
    return &events_[index];
}


#endif // defined(__APPLE__)