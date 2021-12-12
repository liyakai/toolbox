#include "udp_epoll_network.h"

#ifdef __linux__

#include "epoll_define.h"
#include "src/network/net_imp/udp_socket.h"
#include "time_util.h"

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
    epoll_ctrl_.RunOnce<UdpSocket>();
    if(is_kcp_open_)
    {
        auto current = GetMillSecondTimeStamp();
        // 更新 kcp [TODO: 这里的update存在优化空间]
        for(auto iter : address_to_connect_)
        {
            auto socket = sock_mgr_.GetSocket(iter.second);
            if(nullptr == socket)
            {
                continue;
            }
            socket->KcpUpdate(current);
        }
    }
}

void UdpEpollNetwork::CloseListenInMultiplexing(int32_t socket_id)
{
    epoll_ctrl_.DelEvent(socket_id);
}

UdpSocket* UdpEpollNetwork::GetSocketByUdpAddress(const UdpAddress& udp_address)
{
    auto iter = address_to_connect_.find(udp_address.GetID());
    if(iter != address_to_connect_.end())
    {
        return sock_mgr_.GetSocket(iter->second);
    } else 
    {
        return nullptr;
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

void UdpEpollNetwork::OpenKcpMode()
{
    is_kcp_open_ = true;
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
    new_socket->SetNetwork(this);
    if(false == new_socket->InitNewAccepter(ip, port))
    {
        return 0;
    }
    // KCP
    if(is_kcp_open_)
    {
        new_socket->OpenKcpMode();
    }
    epoll_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    address_to_connect_[new_socket->GetLocalAddressID()] = new_socket->GetConnID();
    return new_socket->GetLocalAddressID();
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
    new_socket->SetNetwork(this);
    if(false == new_socket->InitNewConnecter(ip, port))
    {
        return 0;
    }
    // KCP
    if(is_kcp_open_)
    {
        new_socket->OpenKcpMode();
    }
    epoll_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    uint64_t remote_address_id = new_socket->GetRemoteAddressID();
    address_to_connect_[remote_address_id] = new_socket->GetConnID();
    if(remote_address_id > 0)
    {
        OnConnected(remote_address_id);
    }
    return remote_address_id;
}
void UdpEpollNetwork::OnClose(uint64_t address_id)
{
    auto iter = address_to_connect_.find(address_id);
    if(iter == address_to_connect_.end())
    {
        return;
    }
    auto socket = sock_mgr_.GetSocket(iter->second);
    if(nullptr == socket)
    {
        return;
    }
    socket->Close(ENetErrCode::NET_NO_ERROR);
    address_to_connect_.erase(iter);
}

void UdpEpollNetwork::OnSend(uint64_t address_id, const char* data, std::size_t size)
{
    auto iter = address_to_connect_.find(address_id);
    if(iter == address_to_connect_.end())
    {
        return;
    }
    auto socket = sock_mgr_.GetSocket(iter->second);
    if(nullptr == socket)
    {
        return;
    }
    // KCP
    if(is_kcp_open_)
    {
        socket->KcpSendTo(data,size);
        
    } else 
    {
        socket->SendTo(data, size);
    }
    
}

#endif // __linux__