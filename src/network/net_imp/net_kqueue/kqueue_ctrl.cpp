#include "kqueue_ctrl.h"

#if defined(__APPLE__)


KqueueCtrl::KqueueCtrl(uint32_t max_events)
    : fd_num_(max_events)
{
}

bool KqueueCtrl::CreateKqueue()
{
    kqueue_fd_ = kqueue();
    if (kqueue_fd_ < 0)
    {
        return false;
    }
    events_ = new struct kevent[fd_num_];

    return true;
}
void KqueueCtrl::Destroy()
{
    if (events_ != nullptr)
    {
        delete[] events_;
        events_ = nullptr;
    }
    if (kqueue_fd_ > 0)
    {
        close(kqueue_fd_);
        kqueue_fd_ = -1;
    }
}

int32_t KqueueCtrl::Kevent(int msec)
{
    struct timespec ts;
    ts.tv_nsec = 2 * 1000 * 1000;
    return kevent(kqueue_fd_, nullptr, 0, events_, fd_num_, &ts);
}



#endif // defined(__APPLE__)