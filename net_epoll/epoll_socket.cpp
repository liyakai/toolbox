#include "epoll_socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

EpollSocket::EpollSocket()
{
}

EpollSocket::~EpollSocket()
{
}

void EpollSocket::UnInit()
{
}

void EpollSocket::Reset()
{
}

SockEventType EpollSocket::GetEventType() const
{
    return event_type_;
}

void EpollSocket::SetCtrlAdd(bool value)
{
    is_ctrl_add_ = value;
}
void EpollSocket::UpdateAccept()
{
    int client_fd = Accept();
    if(client_fd <= 0)
    {
        // 连接失败
        return;
    }
    socket_state_ = SocketState::SOCK_STATE_LISTENING;
    event_type_ = SOCKET_EVENT_RECV;
    

}
int EpollSocket::Accept()
{
    int client_fd = 0;  // 客户端套接字
    sockaddr_in addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    memset(&addr, 0 ,addr_len);
    while(true)
    {
        // 接受客户端连接
        client_fd = accept(listen_socket_, (sockaddr*)&addr,&addr_len);
        if(-1 == client_fd && errno != EINTR)
        {
            break;
        }
        //EpollSocket* new_socket = EpollSocketMgr->Alloc();
    }

    return client_fd;
}

void EpollSocket::UpdateEpollEvent(SockEventType event_type, time_t ts)
{
    if(SocketState::SOCK_STATE_INVALIED == socket_state_)
    {
        // socket 已经关闭
        return;
    }
    if((event_type & SOCKET_EVENT_RECV) && (event_type_ & SOCKET_EVENT_RECV))
    {
        if(socket_state_ == SocketState::SOCK_STATE_LISTENING)
        {
            UpdateAccept();
        } else 
        {
            last_recv_ts_ = ts; // 更新最后一次读到数据的时间戳

        }
    }
    if((event_type & SOCKET_EVENT_SEND) && (event_type_ & SOCKET_EVENT_SEND))
    {
        if(socket_state_ == SocketState::SOCK_STATE_CONNECTING)
        {

        } else 
        {

        }
    }

}