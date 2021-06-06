#include "epoll_socket.h"

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