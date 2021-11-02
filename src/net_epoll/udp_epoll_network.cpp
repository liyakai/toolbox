#include "udp_epoll_network.h"
#include "epoll_define.h"
#include "udp_socket.h"

UdpEpollNetwork::UdpEpollNetwork()
     : epoll_ctrl_(MAX_SOCKET_COUNT)
  
{
}

UdpEpollNetwork::~UdpEpollNetwork()
{

}

void UdpEpollNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    INetwork::Init(master, network_type);
    epoll_ctrl_.CreateEpoll();
    sock_mgr_.Init(MAX_SOCKET_COUNT);
}

void UdpEpollNetwork::UnInit()
{
    sock_mgr_.UnInit();
    epoll_ctrl_.Destroy();
    INetwork::UnInit();
}

void UdpEpollNetwork::Update()
{
    INetwork::Update();
    epoll_ctrl_.RunOnce();
}

bool UdpEpollNetwork::IsUdpAddressExist(const UdpAddress& udp_address)
{
    auto iter = address_to_connect_.find(udp_address.GetID());
    if(iter != address_to_connect_.end())
    {
        return true;
    } else 
    {
        return false;
    }
}

void UdpEpollNetwork::AddUdpAddress(const UdpAddress& udp_address, uint32_t conn_id)
{
    address_to_connect_[udp_address.GetID()] = conn_id;
}
void UdpEpollNetwork::DeleteUdpAddress(const UdpAddress& udp_address)
{
    address_to_connect_.erase(udp_address.GetID());
}


uint64_t UdpEpollNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetEpollNetwork(this);
    if(false == new_socket->InitNewAccepter(ip, port))
    {
        return 0;
    }
    epoll_ctrl_.OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, new_socket->GetEventType());
    address_to_connect_[new_socket->GetLocalAddress().GetID()] = new_socket->GetConnID();
    return new_socket->GetConnID();
}
uint64_t UdpEpollNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetEpollNetwork(this);
    if(false == new_socket->InitNewConnecter(ip, port))
    {
        return 0;
    }
    epoll_ctrl_.OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, new_socket->GetEventType());
    address_to_connect_[new_socket->GetRemoteAddress().GetID()] = new_socket->GetConnID();
    return new_socket->GetConnID();
}
void UdpEpollNetwork::OnClose(uint64_t connect_id)
{
    auto socket = sock_mgr_.GetEpollSocket((uint32_t)connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Close(ENetErrCode::NET_NO_ERROR);
}

void UdpEpollNetwork::OnSend(uint64_t connect_id, const char* data, std::size_t size)
{
    auto socket = sock_mgr_.GetEpollSocket(connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->SendTo(data, size, socket->GetRemoteAddress().GetAddress());
}