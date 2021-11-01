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
    for(const auto& buffer : read_buffers_)
    {
        GiveBackObject(buffer);
    }
    for(const auto& buffer : dead_buffers_)
    {
        GiveBackObject(buffer);
    }
}

void UdpSocket::Reset()
{
    read_buffers_.clear();
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
    return true;
}
bool UdpSocket::InitNewConnecter(const std::string& ip, uint16_t port)
{
    Bind(ip,port);
    type_ = UdpType::CONNECTOR;
    remote_address_.SetAddress(ip,port);
    return true;
}

void UdpSocket::SendTo(const char* buffer, std::size_t& length, SocketAddress& address)
{
    write_buffers_.emplace_back(GetObject<Buffer>(buffer, length, address));
}

bool UdpSocket::RecvFrom(char* buffer, std::size_t& length, SocketAddress& address)
{
    if(read_buffers_.empty())
    {
        return false;
    }
    auto front = read_buffers_.front();
    memcpy(buffer, front->data_, front->size_);
    length = front->size_;
    address = front->address_;
    read_buffers_.pop_front();
    dead_buffers_.emplace_back(front);
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
    socket_id_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);
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
        p_udp_network_->OnErrored(0, ENetErrCode::NET_LISTEN_FAILED, errno);
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
            read_buffers_.emplace_back(GetObject<Buffer>(array.data(),size, address));
            //p_udp_network_->OnReceived(GetConnID(), buff_block, len);
        } else 
        {
            break;
        }
    }
}

void UdpSocket::UpdateSend()
{
    for(auto iter = write_buffers_.begin(); iter != write_buffers_.end();)
    {
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

bool UdpSocket::SocketRecv(int32_t socket_fd, char* data, size_t& size, SocketAddress& address)
{
    socklen_t fromlen = sizeof(address);
    auto bytes = recvfrom(socket_fd, data, size, 0, (struct sockaddr*)&address, &fromlen);
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

bool UdpSocket::SocketSend(int32_t socket_fd, const char* data, size_t& size, SocketAddress& address)
{
    auto bytes = sendto(socket_fd, data, size, 0, (struct sockaddr*)&address, sizeof(address));
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