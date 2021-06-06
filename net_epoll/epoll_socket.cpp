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