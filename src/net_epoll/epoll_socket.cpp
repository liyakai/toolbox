#include "epoll_socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/tcp.h> // TCP_NODELAY
#include "epoll_socket_pool.h"
#include "epoll_network.h"

EpollSocket::EpollSocket()
{
}

EpollSocket::~EpollSocket()
{
}
bool EpollSocket::Init(int32_t send_buff_len, int32_t recv_buff_len)
{
    send_buff_len_ = 0 == send_buff_len ? DEFAULT_TCP_BUFFER_SIZE : send_buff_len;
    recv_buff_len_ = 0 == recv_buff_len ? DEFAULT_TCP_BUFFER_SIZE : recv_buff_len;
    return true;
}
void EpollSocket::UnInit()
{
    conn_id_ = INVALID_CONN_ID;
    socket_id_ = -1;
    port_ = 0;

    p_tcp_network_ = nullptr;
    p_sock_pool_ = nullptr;

    socket_state_ = SocketState::SOCK_STATE_INVALIED;
    event_type_ = SOCKET_EVENT_INVALID;
    recv_buff_len_ = 0;
    recv_ring_buffer_.Clear();
    send_ring_buffer_.Clear();
    last_recv_ts_ = 0;
    is_ctrl_add_ = false;
}

void EpollSocket::Reset()
{
    UnInit();
}

void EpollSocket::SetCtrlAdd(bool value)
{
    is_ctrl_add_ = value;
}
void EpollSocket::UpdateAccept()
{
    int32_t client_fd = 0; // 客户端套接字
    sockaddr_in addr;
    socklen_t addr_len = sizeof(sockaddr_in);

    while (true)
    {
        memset(&addr, 0, addr_len);
        // 接受客户端连接
        client_fd = accept(socket_id_, (sockaddr *)&addr, &addr_len);
        if (-1 == client_fd && errno != EINTR)
        {
            break;
        }
        EpollSocket *new_socket = p_sock_pool_->Alloc();
        if (nullptr == new_socket)
        {
            close(client_fd);
            // OnError();
            return;
        }
        InitAccpetSocket(new_socket, client_fd, inet_ntoa(addr.sin_addr), addr.sin_port, send_buff_len_, recv_buff_len_);
        // 通知主线程有新的客户端连接进来
        p_tcp_network_->OnAccepted(new_socket->GetConnID());
        p_tcp_network_->GetEpollCtrl().OperEvent(*new_socket, EpollOperType::EPOLL_OPER_ADD, SOCKET_EVENT_RECV);
    }
    socket_state_ = SocketState::SOCK_STATE_LISTENING;
    event_type_ = SOCKET_EVENT_RECV;
}

void EpollSocket::InitAccpetSocket(EpollSocket *socket, int32_t socket_fd, std::string ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    socket->Init(send_buff_size, recv_buff_size);
    socket->SetSocketMgr(p_sock_pool_);
    socket->SetTcpNetwork(p_tcp_network_);
    socket->SetSocketID(socket_fd);
    socket->SetIP(ip);
    socket->SetPort(port);
    socket->SetState(SocketState::SOCK_STATE_ESTABLISHED);
    socket->SetSockEventType(SOCKET_EVENT_RECV | SOCKET_EVENT_SEND);

    socket->SetNonBlocking(socket_fd);
    socket->SetKeepaliveOff(socket_fd);
    socket->SetLingerOff(socket_fd);
    socket->SetNagleOff(socket_fd);
    socket->SetTcpBuffSize(socket_fd);
}

void EpollSocket::UpdateRecv()
{
    size_t cur_buffer_size = recv_ring_buffer_.GetBufferSize();
    if (cur_buffer_size > MAX_RING_BUFF_SIZE_FACTOR * recv_buff_len_)
    {
        // 读缓冲区超过最大限制, error
        Close(ENetErrCode::NET_RECV_BUFF_OVERFLOW);
        return;
    }
    while (size_t size = recv_ring_buffer_.ContinuouslyWriteableSize())
    {
        int32_t bytes = SocketRecv(socket_id_, recv_ring_buffer_.GetWritePtr(), size);
        if (bytes < 0)
        {
            Close(ENetErrCode::NET_RECV_FAILED);
            return;
        }
        else if (0 == bytes)
        {
            break;
        }
        else
        {
            // fprintf(stderr, "接收数据size:%d,buff连续可写的空间:%zu\n", bytes, size);
            recv_ring_buffer_.AdjustWritePos(bytes);
            // fprintf(stderr, "接收数据size:%d,buff连续可写的空间:%zu\n", bytes, recv_ring_buffer_.ContinuouslyWriteableSize());
            // recv_ring_buffer_.DebugPrint(true,false);
        }
        ProcessRecvData();
    }
}

int32_t EpollSocket::SocketRecv(int32_t socket_fd, char *data, size_t size)
{
    int32_t recv_bytes = recv(socket_fd, data, (int32_t)size, MSG_NOSIGNAL);
    if (recv_bytes > 0)
    {
        return recv_bytes;
    }
    else if (0 == recv_bytes)
    {
        return -1;
    }
    else // recv_bytes < 0
    {
        if ((0 == errno) || (EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
}

void EpollSocket::ProcessRecvData()
{
    auto data_size = recv_ring_buffer_.ReadableSize();
    // 这里暂时采用 len|buff 的方式[len包含len自身的长度]分割数据.可重构为传入解包方法
    if (data_size < sizeof(uint32_t))
    {
        return;
    }
    int32_t len = 0;
    recv_ring_buffer_.Copy((char *)&len, sizeof(size_t));
    if (static_cast<int32_t>(data_size) < len)
    {
        return;
    }
    char *buff_block = MemPoolMgr->GetMemory(len);
    recv_ring_buffer_.Read(buff_block, len);
    // fprintf(stderr, "处理数据size:%d\n", len);
    // recv_ring_buffer_.DebugPrint(true,false);
    p_tcp_network_->OnReceived(GetConnID(), buff_block, len);
}

void EpollSocket::UpdateConnect()
{
    event_type_ |= SOCKET_EVENT_RECV;
    p_tcp_network_->GetEpollCtrl().OperEvent(*this, EpollOperType::EPOLL_OPER_ADD, SOCKET_EVENT_RECV);
    socket_state_ = SocketState::SOCK_STATE_ESTABLISHED;
    p_tcp_network_->OnConnected(GetConnID());
}

void EpollSocket::UpdateSend()
{
    while (size_t size = send_ring_buffer_.ContinuouslyReadableSize())
    {
        int32_t bytes = SocketSend(socket_id_, send_ring_buffer_.GetReadPtr(), size);
        if (bytes < 0)
        {
            // 发送失败
            Close(ENetErrCode::NET_SEND_FAILED);
        }
        send_ring_buffer_.AdjustReadPos(bytes);
        if (bytes < static_cast<int32_t>(size))
        {
            break;
        }
    }
}

int32_t EpollSocket::SocketSend(int32_t socket_fd, const char *data, size_t size)
{
    int32_t send_bytes = send(socket_fd, data, (int32_t)size, MSG_NOSIGNAL);
    if (send_bytes < 0)
    {
        if ((0 == errno) || (EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if (0 == send_bytes && size)
    {
        return -1;
    }
    // fprintf(stderr, "发送数据size:%d\n", send_bytes);
    return send_bytes;
}

void EpollSocket::UpdateError()
{
    if(SocketState::SOCK_STATE_ESTABLISHED == socket_state_)
    {
        Close(ENetErrCode::NET_SYS_ERROR, GetSocketError());
    } else 
    {
        p_tcp_network_->OnConnectedFailed(ENetErrCode::NET_CONNECT_FAILED, GetSocketError());
    }
}

int32_t EpollSocket::GetSocketError()
{
    int32_t error = 0;
    socklen_t len = sizeof(error);
    if(getsockopt(socket_id_, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
    {
        return 0;
    }
    return error;
}

void EpollSocket::Close(ENetErrCode net_err, int32_t sys_err)
{
    if (-1 != socket_id_)
    {
        // 通知主线程 socket 关闭
        p_tcp_network_->OnClosed((uint64_t)GetConnID(),(int32_t)net_err, sys_err);
        close(socket_id_);
        socket_id_ = -1;
        socket_state_ = SocketState::SOCK_STATE_INVALIED;
        p_sock_pool_->Free(this);
    }
}

void EpollSocket::UpdateEpollEvent(SockEventType event_type, time_t ts)
{
    if (SocketState::SOCK_STATE_INVALIED == socket_state_)
    {
        // socket 已经关闭
        return;
    
    }
    if(event_type & SOCKET_EVENT_ERR)
    {
        UpdateError();
    }
    if ((event_type & SOCKET_EVENT_RECV) && (event_type_ & SOCKET_EVENT_RECV))
    {
        if (SocketState::SOCK_STATE_LISTENING == socket_state_)
        {
            UpdateAccept();
        }
        else
        {
            last_recv_ts_ = ts; // 更新最后一次读到数据的时间戳
            UpdateRecv();
        }
    }
    if ((event_type & SOCKET_EVENT_SEND) && (event_type_ & SOCKET_EVENT_SEND))
    {
        if (SocketState::SOCK_STATE_CONNECTING == socket_state_)
        {
            UpdateConnect();
        }
        else
        {
            UpdateSend();
        }
    }
}

bool EpollSocket::InitNewAccepter(const std::string &ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    if (SocketState::SOCK_STATE_LISTENING == socket_state_)
    {
        return false;
    }

    send_buff_len_ = send_buff_size;
    recv_buff_len_ = recv_buff_size;

    socket_id_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id_ < 0)
    {
        return false;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_addr.s_addr = inet_addr(ip.c_str());
    SetReuseAddrOn(socket_id_); // 复用端口
    SetLingerOff(socket_id_);   // 立即关闭该连接
    SetDeferAccept(socket_id_); // 1s 之内没有数据发送，则直接关闭连接
    SetNonBlocking(socket_id_); // 设置为非阻塞
    // 绑定端口
    int32_t error = bind(socket_id_, (struct sockaddr *)&sa, sizeof(struct sockaddr));
    if (error < 0)
    {
        return false;
    }
    error = listen(socket_id_, DEFAULT_BACKLOG_SIZE);
    if (error < 0)
    {
        return false;
    }
    socket_state_ = SocketState::SOCK_STATE_LISTENING;
    event_type_ = SOCKET_EVENT_RECV;
    return true;
}

bool EpollSocket::InitNewConnecter(const std::string &ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
{
    if (SocketState::SOCK_STATE_CONNECTING == socket_state_)
    {
        return false;
    }

    send_buff_len_ = send_buff_size;
    recv_buff_len_ = recv_buff_size;

    socket_id_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id_ < 0)
    {
        return false;
    }

    // 设置对端地址
    ip_ = ip;
    // 设置对端端口
    port_ = port;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ip.c_str());
    SetNonBlocking(socket_id_); // 设置为非阻塞
    SetNagleOff(socket_id_);    // 关闭 Nagle
    SetTcpBuffSize(socket_id_);
    int32_t error = connect(socket_id_, (struct sockaddr *)&sa, sizeof(struct sockaddr));
    if (error < 0 && EINPROGRESS != errno && EINTR != errno && EISCONN != error)
    {
        // 通知 主线程连接失败
        p_tcp_network_->OnConnectedFailed(ENetErrCode::NET_SYS_ERROR, errno);
        Close(ENetErrCode::NET_SYS_ERROR, errno);
        return false;
    }
    socket_state_ = SocketState::SOCK_STATE_CONNECTING;
    event_type_ = SOCKET_EVENT_SEND;
    return true;
}

void EpollSocket::Send(const char* data, size_t len)
{
    if(nullptr == data || 0 == len)
    {
        return;
    }

    if(false == send_ring_buffer_.Empty())
    {
        size_t write_size = send_ring_buffer_.Write(data, len);
        if(write_size < len || send_ring_buffer_.GetBufferSize() > MAX_RING_BUFF_SIZE_FACTOR * send_buff_len_)
        {
            Close(ENetErrCode::NET_SEND_BUFF_OVERFLOW);
            return;
        }
        send_ring_buffer_.Write(data, len);
        UpdateSend();
        return;
    }

    int32_t sended = SocketSend(GetSocketID(), data, len);
    if(-1 == sended)
    {
        Close(ENetErrCode::NET_SYS_ERROR, errno);
        return;
    } else if((int32_t)len == sended)
    {
        return;
    } else if ((int32_t)len > sended)
    {
        send_ring_buffer_.Write(data + sended, len - sended);
    }   
    
}

int32_t EpollSocket::SetNonBlocking(int32_t fd)
{
    int32_t flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
int32_t EpollSocket::SetKeepaliveOff(int32_t fd)
{
    int32_t keepalive = 0;
    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive, sizeof(keepalive));
}
int32_t EpollSocket::SetNagleOff(int32_t fd)
{
    int32_t nodelay = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay));
}
// SOCKET在close时候不等待缓冲区发送完成
int32_t EpollSocket::SetLingerOff(int32_t fd)
{
    linger so_linger;
    so_linger.l_onoff = 0;
    return setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
}
/*
这个套接字选项通知内核，如果端口忙，但TCP状态位于 TIME_WAIT ，可以重用端口。如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息，指明"地址已经使用中"。
如果你的服务程序停止后想立即重启，而新套接字依旧使用同一端口，此时SO_REUSEADDR 选项非常有用
*/
int32_t EpollSocket::SetReuseAddrOn(int32_t fd)
{
    int32_t reuse_addr = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_addr, sizeof(reuse_addr));
}

int32_t EpollSocket::SetDeferAccept(int32_t fd)
{
    int32_t secs = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &secs, sizeof(secs));
}

int32_t EpollSocket::SetTcpBuffSize(int32_t fd)
{
    if(send_buff_len_ > static_cast<int32_t>(DEFAULT_TCP_BUFFER_SIZE))
    {
        // fprintf(stderr, "设定 send buffer");
        int32_t snd_size = DEFAULT_TCP_BUFFER_SIZE;
        socklen_t optlen = sizeof(snd_size);
        setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&snd_size, optlen);
    }
    if(recv_buff_len_ > static_cast<int32_t>(DEFAULT_TCP_BUFFER_SIZE))
    {
        // fprintf(stderr, "设定 recv buffer");
        int32_t rcv_size = DEFAULT_TCP_BUFFER_SIZE;
        socklen_t optlen = sizeof(rcv_size);
        setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&rcv_size, optlen);
    }
    return 0;
}

// TCP之Nagle算法&&延迟ACK

/*
1. Nagle算法：

是为了减少广域网的小分组数目，从而减小网络拥塞的出现；

该算法要求一个tcp连接上最多只能有一个未被确认的未完成的小分组，在该分组ack到达之前不能发送其他的小分组，tcp需要收集这些少量的分组，并在ack到来时以一个分组的方式发送出去；其中小分组的定义是小于MSS的任何分组；

该算法的优越之处在于它是自适应的，确认到达的越快，数据也就发哦送的越快；而在希望减少微小分组数目的低速广域网上，则会发送更少的分组；

2. 延迟ACK：

如果tcp对每个数据包都发送一个ack确认，那么只是一个单独的数据包为了发送一个ack代价比较高，所以tcp会延迟一段时间，如果这段时间内有数据发送到对端，则捎带发送ack，如果在延迟ack定时器触发时候，发现ack尚未发送，则立即单独发送；

延迟ACK好处：

(1) 避免糊涂窗口综合症；

(2) 发送数据的时候将ack捎带发送，不必单独发送ack；

(3) 如果延迟时间内有多个数据段到达，那么允许协议栈发送一个ack确认多个报文段；

3. 当Nagle遇上延迟ACK：

试想如下典型操作，写-写-读，即通过多个写小片数据向对端发送单个逻辑的操作，两次写数据长度小于MSS，当第一次写数据到达对端后，对端延迟ack，不发送ack，而本端因为要发送的数据长度小于MSS，所以nagle算法起作用，数据并不会立即发送，而是等待对端发送的第一次数据确认ack；这样的情况下，需要等待对端超时发送ack，然后本段才能发送第二次写的数据，从而造成延迟；

4. 关闭Nagle算法：

使用TCP套接字选项TCP_NODELAY可以关闭套接字选项;

如下场景考虑关闭Nagle算法：

(1) 对端不向本端发送数据，并且对延时比较敏感的操作；这种操作没法捎带ack；

(2) 如上写-写-读操作；对于此种情况，优先使用其他方式，而不是关闭Nagle算法：

--使用writev，而不是两次调用write，单个writev调用会使tcp输出一次而不是两次，只产生一个tcp分节，这是首选方法；

--把两次写操作的数据复制到单个缓冲区，然后对缓冲区调用一次write；

--关闭Nagle算法，调用write两次；有损于网络，通常不考虑；
*/