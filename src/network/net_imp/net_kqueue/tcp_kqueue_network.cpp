#include "tcp_kqueue_network.h"
#include "kqueue_define.h"

#if defined(__APPLE__)

TcpKqueueNetwork::TcpKqueueNetwork()
     : kqueue_ctrl_(MAX_SOCKET_COUNT)
{
}

TcpKqueueNetwork::~TcpKqueueNetwork()
{

}

void TcpKqueueNetwork::Init(NetworkMaster* master, NetworkType network_type)
{
    INetwork::Init(master, network_type);
    kqueue_ctrl_.CreateKqueue();
    sock_mgr_.Init(MAX_SOCKET_COUNT);
}

void TcpKqueueNetwork::UnInit()
{
    sock_mgr_.UnInit();
    kqueue_ctrl_.Destroy();
    INetwork::UnInit();
}

void TcpKqueueNetwork::Update()
{
    INetwork::Update();
    kqueue_ctrl_.RunOnce<TcpSocket>();
}

void TcpKqueueNetwork::CloseListenInMultiplexing(int32_t socket_id)
{
    auto* socket = sock_mgr_.GetSocket(socket_id);
    if (nullptr != socket)
    {
        socket->SetSockEventType(0);
        kqueue_ctrl_.OperEvent(*socket, EventOperType::EVENT_OPER_RDC, socket->GetEventType());
    }
    

}

uint64_t TcpKqueueNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
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
    kqueue_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    return new_socket->GetConnID();
}
uint64_t TcpKqueueNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
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
    kqueue_ctrl_.OperEvent(*new_socket, EventOperType::EVENT_OPER_ADD, new_socket->GetEventType());
    return new_socket->GetConnID();
}
void TcpKqueueNetwork::OnClose(uint64_t connect_id)
{
    auto socket = sock_mgr_.GetSocket((uint32_t)connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Close(ENetErrCode::NET_NO_ERROR);
}

void TcpKqueueNetwork::OnSend(uint64_t connect_id, const char* data, std::size_t size)
{
    auto socket = sock_mgr_.GetSocket(connect_id);
    if(nullptr == socket)
    {
        return;
    }
    socket->Send(data, size);
}


#endif // defined(__APPLE__)