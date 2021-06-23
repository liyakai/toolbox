#include "epoll_network.h"
#include "net_epoll/epoll_define.h"


TcpNetwork::TcpNetwork()
     : epoll_ctrl_(MAX_SOCKET_COUNT)
  
{
}

TcpNetwork::~TcpNetwork()
{

}

void TcpNetwork::Init(NetworkMaster* master)
{
    INetwork::Init(master);
    epoll_ctrl_.CreateEpoll();
    sock_mgr_.Init(MAX_SOCKET_COUNT);
}

void TcpNetwork::UnInit()
{

}

void TcpNetwork::Update()
{
    epoll_ctrl_.RunOnce();
}


uint64_t TcpNetwork::OnNewAccepter(const std::string& ip, const uint16_t port)
{
    EpollSocket* new_socket = sock_mgr_.Alloc();
    if(nullptr != new_socket)
    {
        return 0;
    }
    if(true == new_socket->InitNewAccepter(ip, port))
    {
        return new_socket->GetConnID();
    }
    return 0;
}
uint64_t TcpNetwork::OnNewConnecter(const std::string& ip, const uint16_t port)
{
    return 0;
}
void TcpNetwork::OnClose(uint64_t connect_id)
{

}

void TcpNetwork::OnSend(uint64_t connect_id, const char* data, uint32_t size)
{

}