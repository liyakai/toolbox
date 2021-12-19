#include "kqueue_ctrl.h"

#if defined(__APPLE__)


KqueueCtrl::KqueueCtrl(uint32_t max_events)
    : fd_num_(max_events)
{
}

bool KqueueCtrl::CreateIOMultiplexing()
{
    kqueue_fd_ = kqueue();
    if (kqueue_fd_ < 0)
    {
        return false;
    }
    events_ = new struct kevent[fd_num_];

    return true;
}
void KqueueCtrl::DestroyIOMultiplexing()
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

bool KqueueCtrl::OperEvent(BaseSocket &socket, EventOperType op_type, int32_t event_type)
{
    struct kevent changes[2];    // 要监视的事件列表
    int32_t change_num = 0;
    if (event_type & SOCKET_EVENT_RECV)
    {
        if (EventOperType::EVENT_OPER_ADD == op_type)
        {
            EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, &socket);
        } else if (EventOperType::EVENT_OPER_RDC == op_type)
        {
            EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_READ, EV_DELETE|EV_DISABLE, 0, 0, &socket);
        }
    }
    if (event_type & SOCKET_EVENT_SEND)
    {
        if (EventOperType::EVENT_OPER_ADD == op_type)
        {
            EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, &socket);
        }else if (EventOperType::EVENT_OPER_RDC == op_type)
        {
            EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_READ, EV_DELETE|EV_DISABLE, 0, 0, &socket);
        }
    }

    kevent(kqueue_fd_, changes, change_num, nullptr, 0, nullptr);
    
    return true;
}
bool KqueueCtrl::RunOnce()
{
    time_t time_stamp = time(0);    // 时间戳
    int32_t count = Kevent(KQUEUE_WAIT_MSECONDS);
    if (count < 0)
    {
        return false;
    }
    for (int32_t i = 0; i < count; i++)
    {
        auto& event = events_[i];
        auto* socket = static_cast<BaseSocket*>(event.udata);
        if (nullptr == socket) continue;
        if (event.flags & EV_ERROR)
        {
            socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
        }
        if (event.filter & EVFILT_READ)
        {
            socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        }
        if (event.filter & EVFILT_WRITE)
        {
            socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        }
    }
    return true;
}
int32_t KqueueCtrl::Kevent(int msec)
{
    struct timespec ts;
    ts.tv_nsec = 2 * 1000 * 1000;
    return kevent(kqueue_fd_, nullptr, 0, events_, fd_num_, &ts);
}



#endif // defined(__APPLE__)