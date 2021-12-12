#pragma once

#if defined(__APPLE__)
#include "src/network/net_imp/net_imp_define.h"

/*
* 定义 kqueue 管理类
*/
class KqueueCtrl
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    KqueueCtrl(uint32_t max_events);
    /*
    * 创建 Kqueue
    * @return 是否成功
    */
    bool CreateKqueue();
    /*
    * 销毁 Kqueue
    */
    void Destroy();
    /*
    * 处理事件
    */
    template<typename SocketType>
    bool OperEvent(SocketType &socket, EventOperType op_type, int32_t event_type)
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

        kevent(kqueue_fd_, changes, 0, nullptr, 0, nullptr);

        return true;
    }
    /*
    * @brief kevent 获取结果
    * @param msec 等待毫秒数
    * @return 有读写通知的文件描述符的个数
    */
    int32_t Kevent(int msec);
    /*
    * 执行一次 epoll wait
    */
    template<typename SocketType>
    bool RunOnce()
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
            SocketType* socket = static_cast<SocketType*>(event.udata);
            if (nullptr == socket) continue;
            if ((event.flags & EV_EOF) || (event.flags & EV_ERROR))
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
private:
    uint32_t fd_num_ = 0;       // 最大文件描述符监视数
    int kqueue_fd_ = 0;         // kqueue 文件描述符
    struct kevent *events_;     // kevent返回的事件列表
};


#endif // defined(__APPLE__)