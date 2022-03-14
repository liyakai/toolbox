#pragma once

#ifdef __linux__

#include "src/tools/singleton.h"
#include "src/network/net_imp/net_imp_define.h"
#include "src/network/net_imp/base_ctrl.h"
#include "liburing.h"


namespace ToolBox{

struct IOUringConnInfo {
    uint32_t fd;
    uint16_t type;
    uint16_t bid;
};

/*
* io_uring 控制类
*/
class IOUringCtrl : public IOMultiplexingInterface
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    IOUringCtrl(uint32_t max_events);
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
    // bool DelEvent(int socket_fd);
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
    int32_t EpollWait(int msec);
    /*
    * 执行一次 epoll wait
    */
    bool RunOnce() override
    {
        // time_t time_stamp = time(0);    // 时间戳
        int32_t count = EpollWait(URING_WAIT_MSECONDS);
        if (count < 0)
        {
            return false;
        }
        // io_uring 设置了两个ringbuffer：
        // sq(submission queue):存放提交的IO请求,应用层为生产者操作tail,内核为消费者操作head.其中的entry称为sqe.
        // cq(completion queue):存放处理完成的IO请求,内核为生产者操作tail,应用层为消费者操作head.其中的entry称为cqe.
        struct io_uring_cqe *cqe;
        unsigned head;
        unsigned cqe_count = 0;

        // go through all CQEs
        io_uring_for_each_cqe(&ring_, head, cqe) {
            ++cqe_count;
            IOUringConnInfo conn_i;
            memcpy(&conn_i, &cqe->user_data, sizeof(conn_i));
        }

        // for (int32_t i = 0; i < count; i++)
        // {
        //     epoll_event& event = events_[i];
        //     BaseSocket* socket = static_cast<BaseSocket*>(event.data.ptr);
        //     if (nullptr == socket) continue;
        //     if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
        //     {
        //         socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
        //         epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);
        //     }
        //     if (event.events & EPOLLIN)
        //     {
        //         socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        //     }
        //     if (event.events & EPOLLOUT)
        //     {
        //         socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        //     }
        // }
        return true;
    }
private:
    uint32_t max_events_ = 0; // 最大事件数
    // int epoll_fd_;            // epoll 文件描述符
    // epoll_event *events_;     // epoll 事件数组
    struct io_uring ring_;       // io_uring 全局句柄
    char bufs[BUFFERS_COUNT][MAX_MESSAGE_LEN] = {0};
    int group_id_ = 1337;
};

#define EpollCtrlMgr Singleton<ToolBox::EpollCtrl>::Instance(10240)



};  // ToolBox

#endif // __linux__
