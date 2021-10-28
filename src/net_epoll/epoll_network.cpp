#include "epoll_network.h"
#include "epoll_define.h"


EpollNetwork::EpollNetwork()
     : epoll_ctrl_(MAX_SOCKET_COUNT)
  
{
}

EpollNetwork::~EpollNetwork()
{

}

void EpollNetwork::Init(NetworkMaster* master)
{
    INetwork::Init(master);
    epoll_ctrl_.CreateEpoll();
    sock_mgr_.Init(MAX_SOCKET_COUNT);
}

void EpollNetwork::UnInit()
{
    sock_mgr_.UnInit();
    epoll_ctrl_.Destroy();
    INetwork::UnInit();
}

void EpollNetwork::Update()
{
    INetwork::Update();
    epoll_ctrl_.RunOnce();
}


uint64_t EpollNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetEpollNetwork(this);
    if(false == new_socket->InitNewAccepter(ip, port, send_buff_size, recv_buff_size))
    {
        return 0;
    }
    epoll_ctrl_.OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, new_socket->GetEventType());
    return new_socket->GetConnID();
}
uint64_t EpollNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetEpollNetwork(this);
    if(false == new_socket->InitNewConnecter(ip, port, send_buff_size, recv_buff_size))
    {
        return 0;
    }
    epoll_ctrl_.OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, new_socket->GetEventType());
    return 0;
}
void EpollNetwork::OnClose(uint64_t connect_id)
{
    auto socket = sock_mgr_.GetEpollSocket((uint32_t)connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Close(ENetErrCode::NET_NO_ERROR);
}

void EpollNetwork::OnSend(uint64_t connect_id, const char* data, uint32_t size)
{
    auto socket = sock_mgr_.GetEpollSocket(connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Send(data, size);
}