#include "tcp_iocp_network.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "src/network/net_imp/net_imp_define.h"


TcpIocpNetwork::TcpIocpNetwork()
{
}

TcpIocpNetwork::~TcpIocpNetwork()
{

}

void TcpIocpNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    ImpNetwork<TcpSocket>::Init(master, network_type);
    iocp_ctrl_.CreateIocp();
}

void TcpIocpNetwork::UnInit()
{
    iocp_ctrl_.Destroy();
    ImpNetwork<TcpSocket>::UnInit();
}

void TcpIocpNetwork::Update()
{
    ImpNetwork<TcpSocket>::Update();
    iocp_ctrl_.RunOnce<TcpSocket>();
}


uint64_t TcpIocpNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto conn_id = ImpNetwork<TcpSocket>::OnNewAccepter(ip, port, send_buff_size, recv_buff_size);
    auto* new_socket = sock_mgr_.GetSocket(conn_id);
    if (nullptr != new_socket)
    {
        iocp_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    }
    return INVALID_CONN_ID;
}
uint64_t TcpIocpNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto conn_id = ImpNetwork<TcpSocket>::OnNewConnecter(ip, port, send_buff_size, recv_buff_size);
    auto* new_socket = sock_mgr_.GetSocket(conn_id);
    if (nullptr != new_socket)
    {
        iocp_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
        return new_socket->GetConnID();
    }
    return INVALID_CONN_ID;
}
#endif // __linux__
