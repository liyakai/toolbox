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
    INetwork::Init(master, network_type);
    iocp_ctrl_.CreateIocp();
    sock_mgr_.Init(MAX_SOCKET_COUNT);
}

void TcpIocpNetwork::UnInit()
{
    sock_mgr_.UnInit();
    iocp_ctrl_.Destroy();
    INetwork::UnInit();
}

void TcpIocpNetwork::Update()
{
    INetwork::Update();
    iocp_ctrl_.RunOnce<TcpSocket>();
}


uint64_t TcpIocpNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetNetwork(this);
    if(false == new_socket->InitNewAccepter(ip, port, send_buff_size, recv_buff_size))
    {
        return 0;
    }
    new_socket->SetSocketState(SocketState::SOCK_STATE_LISTENING);
    iocp_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    return new_socket->GetConnID();
}
uint64_t TcpIocpNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    auto new_socket = sock_mgr_.Alloc();
    if(nullptr == new_socket)
    {
        OnErrored(0, ENetErrCode::NET_ALLOC_FAILED, 0);
        return 0;
    }
    new_socket->SetSocketMgr(&sock_mgr_);
    new_socket->SetNetwork(this);
    if(false == new_socket->InitNewConnecter(ip, port, send_buff_size, recv_buff_size))
    {
        return 0;
    }
    new_socket->SetSocketState(SocketState::SOCK_STATE_CONNECTING);
    iocp_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    return new_socket->GetConnID();
}
void TcpIocpNetwork::OnClose(uint64_t connect_id)
{
    auto socket = sock_mgr_.GetEpollSocket((uint32_t)connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Close(ENetErrCode::NET_NO_ERROR);
}

void TcpIocpNetwork::OnSend(uint64_t connect_id, const char* data, std::size_t size)
{
    auto socket = sock_mgr_.GetEpollSocket(connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Send(data, size);
}

#endif // __linux__
