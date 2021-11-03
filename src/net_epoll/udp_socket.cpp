#include "udp_socket.h"
#include <src/tools/object_pool.h>


UdpAddress::UdpAddress(const std::string& ip, uint16_t port)
{
    SetAddress(ip, port);
}
UdpAddress::UdpAddress(const SocketAddress& address)
{
    SetAddress(address);
}
void UdpAddress::Reset()
{
    id_ = 0;
    memset(&address_, 0, sizeof(address_));
}
void UdpAddress::SetAddress(const SocketAddress& address)
{
    address_ = address;
    uint64_t dword = address.sin_addr.s_addr;
    id_ = dword << 32;
    id_ += (uint64_t) address.sin_port;
}
void UdpAddress::SetAddress(const std::string& ip,  uint16_t port)
{
    memset(&address_, 0, sizeof(SocketAddress));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    address_.sin_addr.s_addr = inet_addr(ip.c_str());
    uint64_t dword = address_.sin_addr.s_addr;
    id_ = dword << 32;
    id_ += (uint64_t) address_.sin_port;
}
SocketAddress& UdpAddress::GetAddress()
{
    return address_;
}
uint64_t UdpAddress::GetID() const
{
    return id_;
}

UdpSocket::Buffer::Buffer(const char* data, std::size_t size, SocketAddress& address)
{
    data_ = buffer_;
    size_ = size;
    address_ = address;
    memmove(buffer_, data, size);
}

UdpSocket::UdpSocket()
{
}
UdpSocket::~UdpSocket()
{
    for(const auto& buffer : write_buffers_)
    {
        GiveBackObject(buffer);
    }
    // for(const auto& buffer : read_buffers_)
    // {
    //     GiveBackObject(buffer);
    // }
    for(const auto& buffer : dead_buffers_)
    {
        GiveBackObject(buffer);
    }
}

void UdpSocket::Reset()
{
    // read_buffers_.clear();
    write_buffers_.clear();
    dead_buffers_.clear();
    remote_address_.Reset();
    local_address_.Reset();
    type_ = UdpType::UNKNOWN;

    p_udp_network_ = nullptr;
    p_sock_pool_ = nullptr;
    BaseSocket::Reset();
}


void UdpSocket::UpdateEpollEvent(SockEventType event_type, time_t ts)
{
    if(event_type & SOCKET_EVENT_ERR)
    {
        UpdateError();
    }
    if ((event_type & SOCKET_EVENT_RECV) && (event_type_ & SOCKET_EVENT_RECV))
    {
        UpdateRecv();
    }
    if ((event_type & SOCKET_EVENT_SEND) && (event_type_ & SOCKET_EVENT_SEND))
    {
        UpdateSend();
    }
}


bool UdpSocket::InitNewAccepter(const std::string& ip, uint16_t port)
{
    Bind(ip,port);
    type_ = UdpType::ACCEPTOR;
    local_address_.SetAddress(ip,port);
    event_type_ = SOCKET_EVENT_RECV;
    return true;
}
bool UdpSocket::InitNewConnecter(const std::string& ip, uint16_t port)
{
    Bind(ip,port);
    type_ = UdpType::CONNECTOR;
    remote_address_.SetAddress(ip,port);
    event_type_ = SOCKET_EVENT_RECV | SOCKET_EVENT_SEND;
    return true;
}

void UdpSocket::SendTo(const char* buffer, std::size_t& length, SocketAddress& address)
{
    if(nullptr == buffer || 0 == length)
    {
        return;
    }
    if(false == write_buffers_.empty())
    {
        write_buffers_.emplace_back(GetObject<Buffer>(buffer, length, address));
        UpdateSend();
        return;
    }
    auto send_length = length;
    auto success = SocketSend(GetSocketID(), buffer, length, address);
    if(success && length)
    {
        if(length < send_length)
        {
            write_buffers_.emplace_back(GetObject<Buffer>(buffer + length, send_length - length, address));
        } 
    } else 
    {
        Close(ENetErrCode::NET_SYS_ERROR, errno);
    } 
}

bool UdpSocket::RecvFrom(char* buffer, std::size_t& length, SocketAddress& address)
{
    // if(read_buffers_.empty())
    // {
    //     return false;
    // }
    // auto front = read_buffers_.front();
    // memcpy(buffer, front->data_, front->size_);
    // length = front->size_;
    // address = front->address_;
    // read_buffers_.pop_front();
    // dead_buffers_.emplace_back(front);
    return true;
}

UdpType UdpSocket::GetType()
{
    return type_;
}

void UdpSocket::Close(ENetErrCode net_err, int32_t sys_err)
{
    if (IsSocketValid())
    {
        BaseSocket::Close();
        // 通知主线程 socket 关闭
        p_udp_network_->OnClosed((uint64_t)GetConnID(), net_err, sys_err);
        p_sock_pool_->Free(this);
    }
}


bool UdpSocket::Bind(const std::string& ip, uint16_t port)
{
    socket_id_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_id_ < 0)
    {
        p_udp_network_->OnErrored(0, ENetErrCode::NET_CONNECT_FAILED, errno);
        return false;
    }

    SocketAddress sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ip.c_str());
    // 绑定端口
    int32_t error = bind(socket_id_, (struct sockaddr *)&sa, sizeof(struct sockaddr));
    if (error < 0)
    {
        p_udp_network_->OnErrored(socket_id_, ENetErrCode::NET_LISTEN_FAILED, errno);
        return false;
    }
    return true;
}

void UdpSocket::UpdateError()
{
    Close(ENetErrCode::NET_SYS_ERROR, GetSocketError());
}

void UdpSocket::UpdateRecv()
{
    SocketAddress address;
    std::array<char, DEFAULT_CONN_BUFFER_SIZE> array;
    auto size = array.size();
    while(true)
    {
        auto success = SocketRecv(socket_id_, array.data(), size, address);
        if (success && size) {
            auto conn_id = p_udp_network_->GetConnIdByUdpAddress(address);
            if(conn_id >= 0)
            {
                char *buff_block = MemPoolMgr->GetMemory(size);
                memcpy(buff_block, array.data(), size);
                p_udp_network_->OnReceived(conn_id, buff_block, size);
            } else if (UdpType::ACCEPTOR == type_)
            {
                UpdateAccept(address);
            }
        } else 
        {
            break;
        }
    }
}

void UdpSocket::UpdateSend()
{
    int32_t times = 0;
    for(auto iter = write_buffers_.begin(); iter != write_buffers_.end();)
    {
        times++;
        auto buffer = *iter;
        size_t size = buffer->size_;
        auto success = SocketSend(socket_id_, buffer->data_, size, buffer->address_);
        if(success && size)
        {
            if(size == buffer->size_)
            {
                dead_buffers_.emplace_back(buffer);
                iter = write_buffers_.erase(iter);
            } else 
            {
                buffer->size_ -= size;
                buffer->data_ += size;
                break;
            }
        } else 
        {
            break;
        }
    }
}

void UdpSocket::UpdateAccept(const SocketAddress& address)
{
    auto new_socket = p_sock_pool_->Alloc();
    if(nullptr == new_socket)
    {
        p_udp_network_->OnErrored(GetConnID(), ENetErrCode::NET_ALLOC_FAILED, 0);
        return ;
    }
    InitAccpetSocket(new_socket, address);
    // 通知主线程有新的客户端连接进来
    p_udp_network_->OnAccepted(new_socket->GetConnID());
    p_udp_network_->AddUdpAddress(address, new_socket->GetConnID());
    p_udp_network_->GetEpollCtrl().OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, new_socket->GetEventType());
    return;
}

void UdpSocket::InitAccpetSocket(UdpSocket* socket, const SocketAddress& address)
{
    socket->SetSocketID(socket_id_);
    socket->SetRemoteAddress(UdpAddress(address));
    socket->SetSocketMgr(p_sock_pool_);
    socket->SetEpollNetwork(p_udp_network_);
    socket->SetSockEventType(SOCKET_EVENT_RECV | SOCKET_EVENT_SEND);
}

bool UdpSocket::SocketRecv(int32_t socket_fd, char* data, size_t& size,  const SocketAddress& address)
{
    socklen_t fromlen = sizeof(address);
    auto bytes = recvfrom(socket_fd, data, size, MSG_DONTWAIT, (struct sockaddr*)&address, &fromlen);
    if(bytes < 0)
    {
        size = 0;
        if(EWOULDBLOCK == errno)
        {
            return true;
        } else
        {
            return false;
        }
    }
    size = bytes;
    return true;
}

bool UdpSocket::SocketSend(int32_t socket_fd, const char* data, size_t& size, const SocketAddress& address)
{
    auto bytes = sendto(socket_fd, data, size, MSG_DONTWAIT, (struct sockaddr*)&address, sizeof(address));
    if(bytes < 0)
    {
        size = 0;
        if(EWOULDBLOCK == errno)
        {
            return true;
        } else 
        {
            return false;
        }
    }
    size = bytes;
    return true;
}