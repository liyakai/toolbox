#include "udp_socket.h"

#ifdef __linux__

#include "network/net_imp/net_epoll/udp_epoll_network.h"

namespace ToolBox
{

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

    UdpSocket::Buffer::Buffer(const char* data, std::size_t size)
    {
        data_ = buffer_;
        size_ = size;
        memmove(buffer_, data, size);
    }

    UdpSocket::UdpSocket()
    {
    }
    UdpSocket::~UdpSocket()
    {
        Reset();
    }

    void UdpSocket::Reset()
    {

        BaseSocket::Reset();
        for (const auto& buffer : send_list_)
        {
            GiveBackObjectLockFree(buffer);
        }

        send_list_.clear();
        remote_address_.Reset();
        local_address_.Reset();
        type_ = UdpType::UNKNOWN;
        if (nullptr != kcp_)
        {
            ikcp_release(kcp_);
            kcp_ = nullptr;
        }
        p_network_ = nullptr;
        p_sock_pool_ = nullptr;
    }


    void UdpSocket::UpdateEvent(SockEventType event_type, time_t ts)
    {
        if (event_type & SOCKET_EVENT_ERR)
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


    bool UdpSocket::InitNewAccepter(uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        Bind(ip, port);
        type_ = UdpType::ACCEPTOR;
        local_address_.SetAddress(ip, port);
        event_type_ = SOCKET_EVENT_RECV | SOCKET_EVENT_ERR;
        SetOpaque(opaque);
        return true;
    }
    bool UdpSocket::InitNewConnecter(uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        Bind();
        type_ = UdpType::CONNECTOR;
        remote_address_.SetAddress(ip, port);
        event_type_ = SOCKET_EVENT_RECV | SOCKET_EVENT_SEND | SOCKET_EVENT_ERR;
        return true;
    }

    void UdpSocket::Send(const char* buffer, std::size_t length)
    {
        if (nullptr == buffer || 0 == length)
        {
            return;
        }
        NetworkLogTrace("[Network][UdpSocket] Send udp data. socket id:%d, conn_id:%llu, len.%zu", GetSocketID(), GetConnID(), length);
        if (false == send_list_.empty())
        {
            send_list_.emplace_back(GET_NET_OBJECT(Buffer, buffer, length));
            UpdateSend();
            return;
        }
        auto send_length = length;
        auto success = SocketSend(GetSocketID(), buffer, length);
        if (success && length)
        {
            if (length < send_length)
            {
                send_list_.emplace_back(GET_NET_OBJECT(Buffer, buffer + length, send_length - length));
            }
        }
        else
        {
            Close(ENetErrCode::NET_SYS_ERROR, errno);
        }
    }

    void UdpSocket::KcpSendTo(const char* buffer, std::size_t length)
    {
        ikcp_send(kcp_, buffer, length);
    }

    UdpType UdpSocket::GetType()
    {
        return type_;
    }

    void UdpSocket::Close(ENetErrCode net_err, int32_t sys_err)
    {
        if (IsSocketValid())
        {
            BaseSocket::Close(net_err, sys_err);
            if (p_network_)
            {
                // 通知主线程 socket 关闭
                if (UdpType::ACCEPTOR == type_)
                {
                    p_network_->OnClosed(GetOpaque(), GetLocalAddressID(), net_err, sys_err);
                    p_network_->CloseListenInMultiplexing(GetSocketID());
                }
                else
                {
                    if (UdpType::CONNECTOR == type_)
                    {
                        p_network_->CloseListenInMultiplexing(GetSocketID());
                    }
                    p_network_->OnClosed(GetOpaque(), GetRemoteAddressID(), net_err, sys_err);
                }
                p_sock_pool_->Free(this);
            }

        }
    }


    bool UdpSocket::Bind(const std::string& ip, uint16_t port)
    {
        socket_id_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_id_ < 0)
        {
            p_network_->OnErrored(GetOpaque(), 0, ENetErrCode::NET_LISTEN_FAILED, errno);
            return false;
        }

        SocketAddress sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        sa.sin_addr.s_addr = inet_addr(ip.c_str());
        // 绑定端口
        int32_t error = bind(socket_id_, (struct sockaddr*)&sa, sizeof(struct sockaddr));
        if (error < 0)
        {
            p_network_->OnErrored(GetOpaque(), socket_id_, ENetErrCode::NET_LISTEN_FAILED, errno);
            return false;
        }
        return true;
    }

    bool UdpSocket::Bind()
    {
        socket_id_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_id_ < 0)
        {
            p_network_->OnErrored(GetOpaque(), 0, ENetErrCode::NET_CONNECT_FAILED, errno);
            return false;
        }

        SocketAddress sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = 0;
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        // 绑定端口
        int32_t error = bind(socket_id_, (struct sockaddr*)&sa, sizeof(SocketAddress));
        if (error < 0)
        {
            p_network_->OnErrored(GetOpaque(), socket_id_, ENetErrCode::NET_LISTEN_FAILED, errno);
            return false;
        }
        return true;
    }

    void UdpSocket::OpenKcpMode()
    {
        kcp_ = ikcp_create(KCP_CONV, this);
        ikcp_setoutput(kcp_, &UdpSocket::Output);
        // 设置 MTU
        ikcp_setmtu(kcp_, KCP_TRANSPORT_MTU);
        // 极速模式,官方推荐
        ikcp_nodelay(kcp_, 1, 10, 2, 1);
        return;
    }

    void UdpSocket::KcpUpdate(std::time_t current)
    {
        if (nullptr != kcp_)
        {
            ikcp_update(kcp_, current);
            UpdateSend();
        }
    }

    void UdpSocket::KcpRecv(const char* buffer, std::size_t length, const UdpAddress&& address)
    {
        // 将收到的数据输入到 kcp
        ikcp_input(kcp_, buffer, length);
        // 从 KCP 返回可靠包
        char out_buffer[DEFAULT_CONN_BUFFER_SIZE];
        auto bytes_size = ikcp_recv(kcp_, out_buffer, sizeof(out_buffer));
        if (bytes_size > 0)
        {
            char* buff_block = GET_NET_MEMORY(bytes_size);
            memcpy(buff_block, out_buffer, bytes_size);
            p_network_->OnReceived(GetOpaque(), UdpAddress(address).GetID(), buff_block, bytes_size);
        }
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
        while (true)
        {
            auto success = SocketRecv(socket_id_, array.data(), size, address);
            if (success && size)
            {
                auto* p_udp_epoll_network = dynamic_cast<UdpEpollNetwork*>(p_network_);
                auto* udp_socket = p_udp_epoll_network->GetSocketByUdpAddress(address);
                if (nullptr == udp_socket)
                {
                    udp_socket = UpdateAccept(address);
                }
                if (nullptr == kcp_)    // 原始 udp 模式
                {
                    char* buff_block = GET_NET_MEMORY(size);
                    memcpy(buff_block, array.data(), size);
                    NetworkLogTrace("[Network][UdpSocket] Receive udp data. socket id:%d, conn_id:%llu, len.%zu", GetSocketID(), GetConnID(), size);
                    p_network_->OnReceived(GetOpaque(), UdpAddress(address).GetID(), buff_block, size);
                }
                else                    // 开启了kcp
                {
                    if (nullptr != udp_socket)
                    {
                        NetworkLogTrace("[Network][UdpSocket] Receive kcp data. socket id:%d, conn_id:%llu, len.%zu", GetSocketID(), GetConnID(), size);
                        udp_socket->KcpRecv(array.data(), size, UdpAddress(address));
                    }

                }
            }
            else
            {
                break;
            }
        }
    }

    void UdpSocket::UpdateSend()
    {
        for (auto iter = send_list_.begin(); iter != send_list_.end();)
        {
            auto buffer = *iter;
            size_t size = buffer->size_;
            auto success = SocketSend(socket_id_, buffer->data_, size);
            if (success && size)
            {
                if (size == buffer->size_)
                {
                    GIVE_BACK_OBJECT(buffer);
                    iter = send_list_.erase(iter);
                }
                else
                {
                    buffer->size_ -= size;
                    buffer->data_ += size;
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    UdpSocket* UdpSocket::UpdateAccept(const SocketAddress& address)
    {
        auto* new_socket = p_sock_pool_->Alloc();
        if (nullptr == new_socket)
        {
            p_network_->OnErrored(GetOpaque(), GetLocalAddressID(), ENetErrCode::NET_ALLOC_FAILED, 0);
            return nullptr;
        }
        InitAccpetSocket(new_socket, address);
        auto* p_udp_epoll_network = dynamic_cast<UdpEpollNetwork*>(p_network_);
        if (p_udp_epoll_network->IsKcpModeOpen())
        {
            new_socket->OpenKcpMode();
        }
        NetworkLogDebug("[Network][UdpEpollNetwork] on new client.address_ip:%s, address_port:%u remote_address_id:%llu, conn_id:%llu", inet_ntoa(address.sin_addr), address.sin_port, new_socket->GetRemoteAddressID(), new_socket->GetConnID());
        // 通知主线程有新的客户端连接进来
        p_udp_epoll_network->OnAccepted(GetOpaque(), new_socket->GetRemoteAddressID());
        p_udp_epoll_network->AddUdpAddress(address, new_socket->GetConnID());
        return new_socket;
    }

    void UdpSocket::InitAccpetSocket(UdpSocket* socket, const SocketAddress& address)
    {
        socket->SetSocketID(socket_id_);
        socket->SetRemoteAddress(UdpAddress(address));
        socket->SetSocketMgr(p_sock_pool_);
        socket->SetNetwork(p_network_);
        socket->SetSockEventType(SOCKET_EVENT_RECV | SOCKET_EVENT_SEND | SOCKET_EVENT_ERR);
        socket->SetType(UdpType::REMOTE);
    }

    bool UdpSocket::SocketRecv(int32_t socket_fd, char* data, size_t& size,  SocketAddress& address)
    {
        socklen_t fromlen = sizeof(address);
        auto bytes = recvfrom(socket_fd, data, size, MSG_DONTWAIT, (struct sockaddr*)&address, &fromlen);
        if (bytes < 0)
        {
            size = 0;
            if (EWOULDBLOCK == errno)
            {
                return true;
            }
            else
            {
                p_network_->OnErrored(GetOpaque(), GetLocalAddressID(), ENetErrCode::NET_SYS_ERROR, errno);
                return false;
            }
        }
        size = bytes;
        return true;
    }

    bool UdpSocket::SocketSend(int32_t socket_fd, const char* data, size_t& size)
    {
        // DebugPrint::PrintfData(data, 32, "KCP_SEND");
        auto bytes = sendto(socket_fd, data, size, MSG_DONTWAIT, (struct sockaddr*) & (remote_address_.GetAddress()), sizeof(SocketAddress));
        if (bytes < 0)
        {
            size = 0;
            if (EWOULDBLOCK == errno)
            {
                return true;
            }
            else
            {
                p_network_->OnErrored(GetOpaque(), GetRemoteAddressID(), ENetErrCode::NET_SYS_ERROR, errno);
                return false;
            }
        }
        size = bytes;
        return true;
    }

    int32_t UdpSocket::Output(const char* buf, int32_t len, ikcpcb* kcp, void* user)
    {
        auto* socket = reinterpret_cast<UdpSocket*>(user);
        socket->Send(buf, len);
        return 0;
    }

};  // ToolBox

#endif // __linux__