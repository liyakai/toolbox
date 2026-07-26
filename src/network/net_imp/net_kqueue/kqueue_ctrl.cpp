#include "kqueue_ctrl.h"
#include <cerrno>

#if defined(__APPLE__)

namespace ToolBox
{

    KqueueCtrl::KqueueCtrl(uint32_t max_events)
        : fd_num_(max_events)
    {
    }

    bool KqueueCtrl::CreateIOMultiplexing()
    {
        SetLastSystemError(0);
        kqueue_fd_ = kqueue();
        if (kqueue_fd_ < 0)
        {
            SetLastSystemError(errno);
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
        if (kqueue_fd_ >= 0)
        {
            close(kqueue_fd_);
            kqueue_fd_ = -1;
        }
    }

    bool KqueueCtrl::OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type)
    {
        SetLastSystemError(0);
        struct kevent changes[2];    // 要监视的事件列表
        int32_t change_num = 0;
        if (event_type & SOCKET_EVENT_RECV)
        {
            if (EventOperType::EVENT_OPER_ADD == op_type)
            {
                EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &socket);
            }
            else if (EventOperType::EVENT_OPER_RDC == op_type)
            {
                EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, &socket);
            }
        }
        if (event_type & SOCKET_EVENT_SEND)
        {
            if (EventOperType::EVENT_OPER_ADD == op_type)
            {
                EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &socket);
            }
            else if (EventOperType::EVENT_OPER_RDC == op_type)
            {
                EV_SET(&changes[change_num++], socket.GetSocketID(), EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, &socket);
            }
        }

        // 无变更时无需下发,避免 kevent 空调用被判失败
        if (0 == change_num)
        {
            return true;
        }
        if (0 != kevent(kqueue_fd_, changes, change_num, nullptr, 0, nullptr))
        {
            SetLastSystemError(errno);
            NetworkLogError("[Network] kevent failed. fd:%d, change_num:%d, errno:%d",
                            socket.GetSocketID(), change_num, errno);
            return false;
        }
        return true;
    }

    bool KqueueCtrl::DelEvent(int socket_fd)
    {
        SetLastSystemError(0);
        const int16_t filters[] = {EVFILT_READ, EVFILT_WRITE};
        for (int16_t filter : filters)
        {
            struct kevent change;
            EV_SET(&change, socket_fd, filter, EV_DELETE, 0, 0, nullptr);
            if (0 != kevent(kqueue_fd_, &change, 1, nullptr, 0, nullptr)
                && ENOENT != errno)
            {
                SetLastSystemError(errno);
                NetworkLogWarn("[Network] delete kqueue event failed. fd:%d filter:%d errno:%d",
                               socket_fd, filter, errno);
                return false;
            }
        }
        return true;
    }
    bool KqueueCtrl::RunOnce(std::time_t time_stamp)
    {
        int32_t count = Kevent(KQUEUE_WAIT_MSECONDS);
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
            auto& event = events_[i];
            auto* socket = static_cast<BaseSocket*>(event.udata);
            if (nullptr == socket)
            {
                continue;
            }
            if (event.flags & EV_ERROR)
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                continue;
            }
            if (event.filter == EVFILT_READ)
            {
                socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
            }
            else if (event.filter == EVFILT_WRITE)
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

};  // ToolBox

#endif // defined(__APPLE__)