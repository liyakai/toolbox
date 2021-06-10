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
{}


uint64_t TcpNetwork::OnNewAccepter(const std::string& ip, const uint16_t port)
{
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