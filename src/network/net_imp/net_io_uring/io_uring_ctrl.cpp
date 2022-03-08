#include "io_uring_ctrl.h"

#ifdef __linux__


IOUringCtrl::IOUringCtrl(uint max_events)
    : max_events_(max_events)
{
}

bool IOUringCtrl::CreateIOMultiplexing()
{
    epoll_fd_ = epoll_create(max_events_);
    if (epoll_fd_ < 0)
    {
        return false;
    }
    events_ = new epoll_event[max_events_];

    return true;
}
void IOUringCtrl::DestroyIOMultiplexing()
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

bool IOUringCtrl::DelEvent(int socket_fd)
{
    epoll_event evt;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd, &evt) == 0;
}

int IOUringCtrl::EpollWait(int msec)
{
    return epoll_wait(epoll_fd_, events_, max_events_, msec);
}

};  // ToolBox

#endif  // __linux__






