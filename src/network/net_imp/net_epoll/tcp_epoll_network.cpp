#include "tcp_epoll_network.h"

#ifdef __linux__

#include "epoll_define.h"


TcpEpollNetwork::TcpEpollNetwork()
     : epoll_ctrl_(MAX_SOCKET_COUNT)
  
{
}

TcpEpollNetwork::~TcpEpollNetwork()
{

}

void TcpEpollNetwork::Init(NetworkMaster* master, NetworkType network_type)
{

    ImpNetwork<TcpSocket>::Init(master, network_type);
    epoll_ctrl_.CreateEpoll();
}

void TcpEpollNetwork::UnInit()
{
    epoll_ctrl_.Destroy();
    ImpNetwork<TcpSocket>::UnInit();
}

void TcpEpollNetwork::Update()
{
    ImpNetwork<TcpSocket>::Update();
    epoll_ctrl_.RunOnce<TcpSocket>();
}

void TcpEpollNetwork::CloseListenInMultiplexing(int32_t socket_id)
{
    epoll_ctrl_.DelEvent(socket_id);
}


uint64_t TcpEpollNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto conn_id = ImpNetwork<TcpSocket>::OnNewAccepter(ip, port, send_buff_size, recv_buff_size);
    auto* new_socket = sock_mgr_.GetSocket(conn_id);
    if(nullptr != new_socket)
    {
        epoll_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    } 
    return INVALID_CONN_ID;
}
uint64_t TcpEpollNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto conn_id = ImpNetwork<TcpSocket>::OnNewConnecter(ip, port, send_buff_size, recv_buff_size);
    auto* new_socket = sock_mgr_.GetSocket(conn_id);
    if(nullptr != new_socket)
    {
        epoll_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    }
    return INVALID_CONN_ID;
}
#endif // __linux__
