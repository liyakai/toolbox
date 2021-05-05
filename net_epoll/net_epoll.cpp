#include "net_epoll.h"
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

NetEpoll::NetEpoll(uint32_t max_events)
    :max_events_(max_events)
{
}

ErrCode NetEpoll::CreateEpoll()
{
    epoll_fd_ = epoll_create(max_events_);
    if(epoll_fd_ < 0)
    {
        return ERR_CREATE_EPOLL_FAILED;
    }
    events_ = (epoll_event*)malloc(sizeof(epoll_event) * max_events_);
    if(nullptr == events_)
    {
        return ERR_MALLOC_FAILED;
    }
    return ERR_SUCCESS;
}
void NetEpoll::Destroy()
{
    close(epoll_fd_);
}

ErrCode NetEpoll::AddEvent(int op, int socket_fd)
{
    epoll_event event;
    memset(&events_, 0, sizeof(event));
    event.data.ptr = nullptr;
    event.events |= EPOLLET;

    epoll_ctl(epoll_fd_, op, socket_fd, &event);
    return ERR_SUCCESS;

}
    

ErrCode NetEpoll::EpollWait(int32_t *count)
{
    if(nullptr == count)
    {
        return ERR_PARAM_NULLPTR;
    }
    *count = epoll_wait(epoll_fd_, events_, max_events_, 1);
    if(*count < 0)
    {
        return ERR_EPOLL_WAIT_FAILED;
    }
    return ERR_SUCCESS;
}





