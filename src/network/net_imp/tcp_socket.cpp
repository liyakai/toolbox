#include "tcp_socket.h"
#include "network/net_imp/net_imp_define.h"
#include "network/network_def_internal.h"
#include "tools/time_util.h"
#include <cstdint>
#include <system_error>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#elif defined(__linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/tcp.h> // TCP_NODELAY

#ifdef LINUX_IO_URING
#include "liburing.h"
#endif  //LINUX_IO_URING

#elif defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#endif

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "network/net_imp/socket_pool.h"
#include "network/net_imp/net_epoll/tcp_epoll_network.h"
#include "network/net_imp/net_iocp/tcp_iocp_network.h"
#include "network/net_imp/net_kqueue/tcp_kqueue_network.h"
#include "imp_network.h"
#include "tools/string_util.h"

namespace ToolBox
{

    TcpSocket::TcpSocket()
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        ResetPerSocket();
#elif defined(__linux__)
#endif
    }

    TcpSocket::~TcpSocket()
    {
    }
    bool TcpSocket::Init(int32_t send_buff_len, int32_t recv_buff_len)
    {
        send_buff_len_ = 0 == send_buff_len ? DEFAULT_CONN_BUFFER_SIZE : send_buff_len;
        recv_buff_len_ = 0 == recv_buff_len ? DEFAULT_CONN_BUFFER_SIZE : recv_buff_len;
        return true;
    }
    void TcpSocket::UnInit()
    {
        Reset();
    }

    void TcpSocket::Reset()
    {
        p_network_ = nullptr;
        p_sock_pool_ = nullptr;
        socket_state_ = SocketState::SOCK_STATE_INVALIED;  // socket 状态
        send_buff_len_ = 0;
        recv_buff_len_ = 0;
        recv_ring_buffer_.Clear();
        send_ring_buffer_.Clear();
        last_recv_ts_ = 0;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        if (nullptr != per_socket_.accept_ex)
        {
            per_socket_.accept_ex->accept_ex_fn = nullptr;
        }
        delete per_socket_.accept_ex;
        per_socket_.accept_ex = nullptr;
        ResetPerSocket();
#elif defined(__linux__)
#if defined (LINUX_IO_URING)
        delete uring_socket_.accept_ex;
        uring_socket_.accept_ex = nullptr;
        ResetUringSocket();
#endif
#endif

        BaseSocket::Reset();
    }

    void TcpSocket::UpdateAccept()
    {
        int32_t client_fd = 0; // 客户端套接字
        SocketAddress addr;

        while (true)
        {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            client_fd = per_socket_.accept_ex->socket_fd;
            SOCKET accept_socket_id = GetSocketID();
            // 链接进入的socket继承 accept_socket 的属性
            setsockopt(client_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&accept_socket_id, sizeof(SOCKET));
            // 获取远端地址
            sockaddr_in* remote_address = GetRemoteAddress(GetSocketID(), per_socket_.accept_ex->buffer, ACCEPTEX_BUFF_SIZE);
            if (nullptr != remote_address)
            {
                addr = *remote_address;
            }
            if (INVALID_SOCKET == client_fd)
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
            client_fd = uring_socket_.accept_ex->socket_fd;
#else
            socklen_t addr_len = sizeof(SocketAddress);
            memset(&addr, 0, addr_len);
            // 接受客户端连接
            client_fd = accept(socket_id_, (sockaddr*)&addr, &addr_len);
#endif

            if (client_fd < 0 && errno != EINTR)
#endif
            {
                break;
            }
            // 通知主线程有新的客户端连接进来
            p_network_->OnAccepting(GetOpaque(), client_fd, inet_ntoa(addr.sin_addr), addr.sin_port, send_buff_len_, recv_buff_len_);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            // 将监听socket重新加入iocp
            ReAddSocketToIocp(SOCKET_EVENT_RECV);
            break;
#elif defined(__linux__)
#if defined (LINUX_IO_URING)
            // 将监听socket重新加入iocp
            ReAddSocketToUring(SOCKET_EVENT_RECV);
            break;
#endif
#elif defined(__APPLE__)
#endif
        }
    }

    bool TcpSocket::InitAccpetSocket(uint64_t opaque, int32_t socket_fd, std::string ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        SetOpaque(opaque);
        Init(send_buff_size, recv_buff_size);
        SetSocketID(socket_fd);
        // SetIP(ip);
        // SetAddressPort(port);
        SetSockEventType(SOCKET_EVENT_RECV | SOCKET_EVENT_SEND);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        SetSocketState(SocketState::SOCK_STATE_ESTABLISHED);
        // 建立 socket 与 iocp 的关联
        if (!AssociateSocketToIocp())
        {
            return false;
        }
#elif defined(__linux__) || defined(__APPLE__)
        SetSocketState(SocketState::SOCK_STATE_ESTABLISHED);
#endif

        SetNonBlocking(socket_fd);
        SetKeepaliveOff(socket_fd);
        SetLingerOff(socket_fd);
        SetNagleOff(socket_fd);
        SetTcpBuffSize(socket_fd);
        return true;
    }

    void TcpSocket::UpdateRecv()
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        recv_ring_buffer_.AdjustWritePos(per_socket_.io_recv.wsa_buf.len);
        // 处理数据
        if (ErrCode::ERR_SUCCESS != ProcessRecvData())
        {
            return;
        }

        // 检查接收 buffer 是否超限
        if (!CheckRecvRingBufferSize())
        {
            return;
        }
        ReAddSocketToIocp(SOCKET_EVENT_RECV);
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
        recv_ring_buffer_.AdjustWritePos(uring_socket_.io_recv.len);
        // 处理数据
        if (ErrCode::ERR_SUCCESS != ProcessRecvData())
        {
            return;
        }

        // 检查接收 buffer 是否超限
        if (!CheckRecvRingBufferSize())
        {
            return;
        }
        ReAddSocketToUring(SOCKET_EVENT_RECV);
#else
        while (size_t size = recv_ring_buffer_.ContinuouslyWriteableSize())
        {
            int32_t bytes = SocketRecv(socket_id_, recv_ring_buffer_.GetWritePtr(), size);
            if (bytes < 0)
            {
                NetworkLogError("[network] tcp socket UpdateRecv. SocketRecv failed. errno:%d.", errno);
                Close(ENetErrCode::NET_RECV_FAILED);
                return;
            }
            else if (0 == bytes)
            {
                sim_nagle_.flag_can_recv = false;           //  模拟nagle 是否可接收置为 false
                break;
            }
            else
            {
                recv_ring_buffer_.AdjustWritePos(bytes);
            }

            // 处理数据
            ErrCode err_code = ProcessRecvData();
            if (ErrCode::ERR_SUCCESS != err_code)
            {
                NetworkLogWarn("[Network] ProcessRecvData.err_code:%d", err_code);
                return;
            }

            // 检查接收 buffer 是否超限
            if (!CheckRecvRingBufferSize())
            {
                return;
            }
        }
#endif

#endif
        return;
    }

    int32_t TcpSocket::SocketRecv(int32_t socket_fd, char* data, size_t size)
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        int32_t recv_bytes = recv(socket_fd, data, (int32_t)size, 0);
#elif defined(__linux__) || defined(__APPLE__)
        int32_t recv_bytes = recv(socket_fd, data, (int32_t)size, MSG_NOSIGNAL);
#endif
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
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            int32_t last_errno = GetSocketError();
            if ((0 == last_errno) || (WSAEINTR == last_errno) || (WSAEINPROGRESS == last_errno) || (WSAEWOULDBLOCK == last_errno))
#elif defined(__linux__) || defined(__APPLE__)
            if ((0 == errno) || (EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
#endif
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
    }

    ErrCode TcpSocket::ProcessRecvData()
    {
        while (true)
        {
            auto data_size = recv_ring_buffer_.ReadableSize();
            // 这里暂时采用 len|buff 的方式[len不包含len自身的长度]分割数据.可重构为传入解包方法
            if (data_size < sizeof(uint32_t))
            {
                // return ErrCode::ERR_INSUFFICIENT_LENGTH;
                break;
            }
            uint32_t len = 0;
            recv_ring_buffer_.Copy((char*)&len, sizeof(uint32_t));
            if (len > static_cast<uint32_t>(recv_buff_len_))
            {
                int readable_size = recv_ring_buffer_.ReadableSize();
                int continue_read_size = recv_ring_buffer_.ContinuouslyReadableSize();
                std::size_t read_pos = recv_ring_buffer_.ReadPos();
                NetworkLogError("[Network][TcpSocket] Packet size is invaliable. socket id:%d, conn_id:%llu, capa:%d, len:%u, readable_size:%d, continue_read_size:%d, read_pos:%zu, buffer:%s, buffer2:%s."
                        , GetSocketID(), GetConnID(), recv_ring_buffer_.GetBufferSize(), len, readable_size, continue_read_size, read_pos, ToolBox::MemoryToStr(recv_ring_buffer_.GetReadPtr(), 45).c_str()
                        , ToolBox::MemoryToStr(recv_ring_buffer_.GetReadPtr()-90, 180).c_str());
                Close(ENetErrCode::NET_INVALID_PACKET_SIZE);
                return ErrCode::ERR_INVALID_PACKET_SIZE;
            }
            else if (data_size - sizeof(uint32_t) < len)
            {
                // return ErrCode::ERR_INSUFFICIENT_LENGTH;
                break;
            }
            recv_ring_buffer_.AdjustReadPos(sizeof(uint32_t));
            char* buff_block = MemPoolLockFreeMgr->GetMemory(len);
            uint32_t readed_len = recv_ring_buffer_.Read(buff_block, len);
            if (readed_len != len)
            {
                NetworkLogError("[Network][TcpSocket] recv_ring_buffer_ data is not enough to read. socket id:%d, conn_id:%llu, len:%u, readed_len:%u", GetSocketID(), GetConnID(), len, readed_len);
                Close(ENetErrCode::NET_INVALID_PACKET_SIZE);
                return ErrCode::ERR_INVALID_PACKET_SIZE;
            }

            p_network_->OnReceived(GetOpaque(), GetConnID(), buff_block, len);
        }

        return ErrCode::ERR_SUCCESS;
    }

    void TcpSocket::UpdateConnect()
    {
        // 完成主动连接后增加可处理"接收"能力.
        event_type_ |= SOCKET_EVENT_RECV;
        socket_state_ = SocketState::SOCK_STATE_ESTABLISHED;

        // TODO delete. 三者形式是统一的,不过先注释掉,没有问题的话就都删掉.
        // #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        //         ReAddSocketToIocp(SOCKET_EVENT_RECV);
        // #elif defined(__linux__)
        //         auto p_epoll_network = dynamic_cast<TcpEpollNetwork*>(p_network_);
        //         p_epoll_network->GetBaseCtrl()->OperEvent(*this, EventOperType::EVENT_OPER_ADD, SOCKET_EVENT_RECV);
        // #elif defined(__APPLE__)
        //         auto p_kqueue_network = dynamic_cast<TcpKqueueNetwork*>(p_network_);
        //         p_kqueue_network->GetBaseCtrl()->OperEvent(*this, EventOperType::EVENT_OPER_ADD, SOCKET_EVENT_RECV);
        // #endif

        auto p_iocp_network = dynamic_cast<ImpNetwork<TcpSocket>*>(p_network_);
        if (nullptr == p_iocp_network)
        {
            return ;
        }
        p_iocp_network->GetBaseCtrl()->OperEvent(*this, EventOperType::EVENT_OPER_ADD, SOCKET_EVENT_RECV);

        p_network_->OnConnected(GetOpaque(), GetConnID());
    }

    void TcpSocket::UpdateSend()
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        send_ring_buffer_.AdjustReadPos(per_socket_.io_send.wsa_buf.len);
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
        send_ring_buffer_.AdjustReadPos(uring_socket_.io_send.len);
#endif
#endif
        size_t size = 0;
        while ((size = send_ring_buffer_.ContinuouslyReadableSize()) != 0)
        {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            ReAddSocketToIocp(SOCKET_EVENT_SEND);
            break;  // 做 io_uring 时觉得这里应该添加一个.
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
            ReAddSocketToUring(SOCKET_EVENT_SEND);
            break;
#else
            int32_t bytes = SocketSend(socket_id_, send_ring_buffer_.GetReadPtr(), size);
            if (bytes < 0)
            {
                // 发送失败
                Close(ENetErrCode::NET_SEND_FAILED);
            }
            send_ring_buffer_.AdjustReadPos(bytes);
            if (bytes < static_cast<int32_t>(size))
            {
                sim_nagle_.flag_can_sent = false;           //  模拟nagle 是否可发送置为 false
                break;
            }
#endif  // LINUX_IO_URING

#endif
        }
        if (0 == send_ring_buffer_.ContinuouslyReadableSize())
        {
            sim_nagle_.num_of_unsent_packets = 0;       //  模拟nagle 计数置为 0
        }
    }

    int32_t TcpSocket::SocketSend(int32_t socket_fd, const char* data, size_t size)
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        int32_t send_bytes = 0; //send(socket_fd, data, (int32_t)size, 0);
        return send_bytes;
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
        int32_t send_bytes = 0;
        return send_bytes;
#else
        int32_t send_bytes = send(socket_fd, data, (int32_t)size, MSG_NOSIGNAL);
#endif

#endif
        if (send_bytes < 0)
        {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            int32_t last_errno = GetSocketError();
            if ((0 == last_errno) || (WSAEINTR == last_errno) || (WSAEINPROGRESS == last_errno) || (WSAEWOULDBLOCK == last_errno))
#elif defined(__linux__) || defined(__APPLE__)
            if ((0 == errno) || (EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
#endif
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
        return send_bytes;
    }

    void TcpSocket::UpdateError()
    {
        if (SocketState::SOCK_STATE_ESTABLISHED == socket_state_)
        {
            NetworkLogError("[Network][TcpSocket] socket id:%d, conn_id:%llu, NET_SYS_ERROR.%d", GetSocketID(), GetConnID(), GetSocketError());
            Close(ENetErrCode::NET_SYS_ERROR, GetSocketError());
        }
        else
        {
            NetworkLogError("[Network][TcpSocket] ConnectedFailed. socket id:%d, conn_id:%llu, NET_SYS_ERROR.%d", GetSocketID(), GetConnID(), GetSocketError());
            p_network_->OnConnectedFailed(GetOpaque(), ENetErrCode::NET_CONNECT_FAILED, GetSocketError());
        }
    }

    bool TcpSocket::CheckRecvRingBufferSize()
    {
        if (recv_ring_buffer_.GetBufferSize() > static_cast<size_t>(recv_buff_len_))
        {
            // 读缓冲区超过最大限制, error
            Close(ENetErrCode::NET_RECV_BUFF_OVERFLOW);
            return false;
        }
        return true;
    }

    bool TcpSocket::CheckSendRingBufferSize()
    {
        if (send_ring_buffer_.GetBufferSize() > static_cast<size_t>(send_buff_len_))
        {
            Close(ENetErrCode::NET_SEND_BUFF_OVERFLOW);
            return false;
        }
        return true;
    }


    void TcpSocket::Close(ENetErrCode net_err, int32_t sys_err)
    {
        if (IsSocketValid())
        {
            NetworkLogDebug("[Network] socket id:%d, conn_id:%llu, Ready to be free.net_err:%d, sys_err:%d", GetSocketID(), GetConnID(), net_err, sys_err);
            BaseSocket::Close(net_err, sys_err);
            p_network_->CloseListenInMultiplexing(GetSocketID());
            // 通知主线程 socket 关闭
            p_network_->OnClosed(GetOpaque(), (uint64_t)GetConnID(), net_err, sys_err);
            socket_state_ = SocketState::SOCK_STATE_INVALIED;
            p_sock_pool_->Free(this);
        }
    }

    void TcpSocket::Update(std::time_t time_stamp)
    {
        if (sim_nagle_.flag_can_sent)
        {
            UpdateSend();
        }
        if (sim_nagle_.flag_can_recv)
        {
            UpdateRecv();
        }
    }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    bool TcpSocket::IsSocketValid()
    {
        if (BaseSocket::IsSocketValid())
        {
            return true;
        }
        if (nullptr != per_socket_.accept_ex
                && per_socket_.accept_ex->socket_fd > 0)
        {
            return true;
        }
        return false;
    }
    sockaddr_in* TcpSocket::GetRemoteAddress(SOCKET&& listen_socket, char* accept_ex_buffer, int32_t buff_len)
    {
        DWORD bytes = 0;
        GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
        LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockAddrs = nullptr;
        // 获取函数指针
        WSAIoctl(
            listen_socket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &GuidGetAcceptExSockAddrs,
            sizeof(GuidGetAcceptExSockAddrs),
            &GetAcceptExSockAddrs,
            sizeof(GetAcceptExSockAddrs),
            &bytes,
            NULL,
            NULL);
        if (nullptr == GetAcceptExSockAddrs)
        {
            return nullptr;
        }
        sockaddr_in* client_addr = NULL;
        SOCKADDR_IN* local_addr = NULL;
        int32_t remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

        GetAcceptExSockAddrs(accept_ex_buffer, (buff_len - ACCEPTEX_ADDR_SIZE * 2), ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, (LPSOCKADDR*)&local_addr, &localLen, (LPSOCKADDR*)&client_addr, &remoteLen);
        return client_addr;
    }

    void TcpSocket::ResetPerSocket()
    {
        ResetRecvAsyncSocket();
        ResetSendAsyncSocket();
    }

    void TcpSocket::ResetRecvAsyncSocket()
    {
        memset(&per_socket_.io_recv, 0, sizeof(per_socket_.io_recv));
        per_socket_.io_recv.wsa_buf.buf = recv_ring_buffer_.GetWritePtr();
        per_socket_.io_recv.wsa_buf.len = recv_ring_buffer_.ContinuouslyWriteableSize();
    }

    void TcpSocket::ResetSendAsyncSocket()
    {
        memset(&per_socket_.io_send, 0, sizeof(per_socket_.io_send));;
        per_socket_.io_send.wsa_buf.buf = send_ring_buffer_.GetReadPtr();
        per_socket_.io_send.wsa_buf.len = send_ring_buffer_.ContinuouslyReadableSize();
    }

    bool TcpSocket::AssociateSocketToIocp()
    {
        auto tcp_iocp_network = dynamic_cast<ImpNetwork<TcpSocket>*>(p_network_);
        // 建立 socket 与 iocp 的关联
        if (nullptr == tcp_iocp_network)
        {
            return false;
        }
        return tcp_iocp_network->GetBaseCtrl()->AssociateSocketToIocp(*this);
    }

    bool TcpSocket::ReAddSocketToIocp(SockEventType event_type)
    {
        auto p_iocp_network = dynamic_cast<ImpNetwork<TcpSocket>*>(p_network_);
        if (nullptr == p_iocp_network)
        {
            return false;
        }
        // 将监听socket重新加入iocp
        return p_iocp_network->GetBaseCtrl()->OperEvent(*this, EventOperType::EVENT_OPER_ADD, event_type);
    }
#elif defined(__linux__) || defined(__APPLE__)



#if defined (LINUX_IO_URING)
    /*
    * 重置 PerSocket
    */
    void TcpSocket:: ResetUringSocket()
    {
        ResetRecvAsyncSocket();
        ResetSendAsyncSocket();
    }
    /*
    * 重置接收 PerSocket
    */
    void TcpSocket::ResetRecvAsyncSocket()
    {
        memset(&uring_socket_.io_recv, 0, sizeof(uring_socket_.io_recv));
        uring_socket_.io_recv.buf = recv_ring_buffer_.GetWritePtr();
        uring_socket_.io_recv.len = recv_ring_buffer_.ContinuouslyWriteableSize();
    }
    /*
    * 重置发送 PerSocket
    */
    void TcpSocket::ResetSendAsyncSocket()
    {
        memset(&uring_socket_.io_send, 0, sizeof(uring_socket_.io_send));;
        uring_socket_.io_send.buf = send_ring_buffer_.GetReadPtr();
        uring_socket_.io_send.len = send_ring_buffer_.ContinuouslyReadableSize();
    }
    bool TcpSocket::ReAddSocketToUring(SockEventType event_type)
    {
        auto p_uring_network = dynamic_cast<ImpNetwork<TcpSocket>*>(p_network_);
        if (nullptr == p_uring_network)
        {
            return false;
        }
        // 将监听socket重新加入iocp
        return p_uring_network->GetBaseCtrl()->OperEvent(*this, EventOperType::EVENT_OPER_ADD, event_type);
    }
#endif



#endif


    void TcpSocket::UpdateEvent(SockEventType event_type, time_t ts)
    {
        if (SocketState::SOCK_STATE_INVALIED == socket_state_)
        {
            // socket 已经关闭
            return;
        }

        if (event_type & SOCKET_EVENT_ERR)
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
                if (p_network_->GetSimulateNagleTimeout() > 0)
                {
                    sim_nagle_.flag_can_recv = true;
                }
                else
                {
                    UpdateRecv();
                }

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
                if (p_network_->GetSimulateNaglePacketsNum() > 0 || p_network_->GetSimulateNagleTimeout() > 0)
                {
                    sim_nagle_.flag_can_sent = true;
                }
                else
                {
                    UpdateSend();
                }

            }
        }
    }

    bool TcpSocket::InitNewAccepter(uint64_t opaque, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        if (SocketState::SOCK_STATE_LISTENING == socket_state_)
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_LISTEN_FAILED, 0);
            return false;
        }
        SetOpaque(opaque);
        Init(send_buff_size, recv_buff_size);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        socket_id_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
        if (socket_id_ == INVALID_SOCKET)
#elif defined(__linux__) || defined(__APPLE__)
        socket_id_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_id_ < 0)
#endif
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_LISTEN_FAILED, GetSysErrNo());
            return false;
        }

        SocketAddress sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        sa.sin_addr.S_un.S_addr = INADDR_ANY;
        if (0 != ip.size())
        {
            sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
        }
#elif defined(__linux__) || defined(__APPLE__)
        sa.sin_addr.s_addr = INADDR_ANY;
        if (0 != ip.size())
        {
            sa.sin_addr.s_addr = inet_addr(ip.c_str());
        }
#endif
        SetReuseAddrOn(socket_id_); // 复用端口
        SetLingerOff(socket_id_);   // 立即关闭该连接
        SetDeferAccept(socket_id_); // 1s 之内没有数据发送，则直接关闭连接
#if !defined(LINUX_IO_URING)
        SetNonBlocking(socket_id_); // 设置为非阻塞
#endif
        // 绑定端口
        int32_t error = bind(socket_id_, (struct sockaddr*)&sa, sizeof(struct sockaddr));
        if (error < 0)
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_LISTEN_FAILED, GetSysErrNo());
            return false;
        }
        error = listen(socket_id_, DEFAULT_BACKLOG_SIZE);
        if (error < 0)
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_LISTEN_FAILED, GetSysErrNo());
            return false;
        }
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // 建立 socket 与 iocp 的关联
        if (!AssociateSocketToIocp())
        {
            return false;
        }
        socket_state_ = SocketState::SOCK_STATE_LISTENING;
#elif defined(__linux__) || defined(__APPLE__)
        socket_state_ = SocketState::SOCK_STATE_LISTENING;
#endif
        // 用于监听的 socket 只能接受"接收","错误"事件.
        event_type_ = SOCKET_EVENT_RECV | SOCKET_EVENT_ERR;
        return true;
    }

    bool TcpSocket::InitNewConnecter(uint64_t opaque, const std::string& ip, uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        if (SocketState::SOCK_STATE_CONNECTING == socket_state_)
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_CONNECT_FAILED, 0);
            return false;
        }
        SetOpaque(opaque);
        Init(send_buff_size, recv_buff_size);

        socket_id_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_id_ < 0)
        {
            p_network_->OnErrored(opaque, 0, ENetErrCode::NET_CONNECT_FAILED, errno);
            return false;
        }

        // // 设置对端地址
        // ip_ = ip;
        // // 设置对端端口
        // port_ = port;
        SocketAddress sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        sa.sin_addr.S_un.S_addr = INADDR_ANY;
        if (0 != ip.size())
        {
            sa.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
        }
#elif defined(__linux__) || defined(__APPLE__)
        sa.sin_addr.s_addr = INADDR_ANY;
        if (0 != ip.size())
        {
            sa.sin_addr.s_addr = inet_addr(ip.c_str());
        }
#endif
        SetNonBlocking(socket_id_); // 设置为非阻塞
        SetNagleOff(socket_id_);    // 关闭 Nagle
        SetTcpBuffSize(socket_id_);
        int32_t error = connect(socket_id_, (struct sockaddr*)&sa, sizeof(struct sockaddr));
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        if (error < 0 && ((WSAEWOULDBLOCK != GetSysErrNo()) && (WSAEISCONN != GetSysErrNo())))
#elif defined(__linux__) || defined(__APPLE__)
        if (error < 0 && EINPROGRESS != errno && EINTR != errno && EISCONN != error)
#endif
        {
            NetworkLogError("[Network][TcpSocket] ConnectedFailed. socket id:%d, conn_id:%llu, NET_SYS_ERROR.%d", GetSocketID(), GetConnID(), GetSocketError());
            // 通知 主线程连接失败
            p_network_->OnConnectedFailed(GetOpaque(), ENetErrCode::NET_SYS_ERROR, GetSysErrNo());
            Close(ENetErrCode::NET_SYS_ERROR, errno);
            return false;
        }
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // 建立 socket 与 iocp 的关联
        if (!AssociateSocketToIocp())
        {
            return false;
        }
#elif defined(__linux__) || defined(__APPLE__)
#endif
        socket_state_ = SocketState::SOCK_STATE_CONNECTING;
        // 用于主动连接的 socket 可接受 "发送","接收","错误" 事件.
        // 在获取正确连接前只接受 "发送","错误"事件.
        event_type_ |= SOCKET_EVENT_SEND | SOCKET_EVENT_ERR;

        return true;
    }

    void TcpSocket::Send(const char* data, size_t len)
    {
        if (nullptr == data || 0 == len)
        {
            return;
        }

        // uint32_t packet_size = 0;
        // memcpy((char*)&packet_size, data, sizeof(uint32_t));
        // NetworkLogTrace("[Network][TcpSocket] Send data size:%u. socket id:%d, conn_id:%llu", packet_size, GetSocketID(), GetConnID());

        if (debug_statistic_save_ + debug_statistic_send_ >= 6000)
        {
            //NetworkLogDebug("[Network] #### debug #### socket_id:%d debug_statistic_save_:%d, debug_statistic_send_:%d, config packet num:%d", GetSocketID(), debug_statistic_save_, debug_statistic_send_, p_network_->GetSimulateNaglePacketsNum());
            debug_statistic_save_ = 0;
            debug_statistic_send_ = 0;
        }
        if (p_network_->GetSimulateNaglePacketsNum() > 0)
        {
            send_ring_buffer_.Write(data, len);
            sim_nagle_.num_of_unsent_packets++; // 模拟nagle 计数增加
            if (sim_nagle_.num_of_unsent_packets < uint32_t(p_network_->GetSimulateNaglePacketsNum()))
            {
                debug_statistic_save_++;
                if (!CheckSendRingBufferSize())
                {
                    return;
                }
                return;
            }
            else
            {
                debug_statistic_send_++;
                UpdateSend();
                return;
            }

        }
        else
        {
            if (false == send_ring_buffer_.Empty())
            {
                send_ring_buffer_.Write(data, len);
                if (!CheckSendRingBufferSize())
                {
                    return;
                }
                return;
            }
            int32_t sended = SocketSend(GetSocketID(), data, len);
            if (-1 == sended)
            {
                NetworkLogError("[Network][TcpSocket] SocketSend failed. socket id:%d, conn_id:%llu, errno:%d, len:%d.", GetSocketID(), GetConnID(), errno, len);
                Close(ENetErrCode::NET_SYS_ERROR, errno);
                return;
            }
            else if ((int32_t)len == sended)
            {
                return;
            }
            else if ((int32_t)len > sended)
            {
                send_ring_buffer_.Write(data + sended, len - sended);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                ReAddSocketToIocp(SOCKET_EVENT_SEND);
#elif defined(__linux__) || defined(__APPLE__)
#if defined (LINUX_IO_URING)
                ReAddSocketToUring(SOCKET_EVENT_SEND);
#endif
#endif
            }
        }
    }

    int32_t TcpSocket::SetNonBlocking(int32_t fd)
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        u_long nonblocking = 1;
        if (INVALID_SOCKET == fd)
        {
            return 1;
        }
        if (SOCKET_ERROR == ioctlsocket(fd, FIONBIO, &nonblocking))
        {
            return 1;
        }
#elif defined(__linux__) || defined(__APPLE__)
        if (fd < 0)
        {
            return 1;
        }
        int32_t flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
        return 0;
    }
    int32_t TcpSocket::SetKeepaliveOff(int32_t fd)
    {
        int32_t keepalive = 0;
        return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive));
    }
    int32_t TcpSocket::SetNagleOff(int32_t fd)
    {
        int32_t nodelay = 1;
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    }
    // SOCKET在close时候不等待缓冲区发送完成
    int32_t TcpSocket::SetLingerOff(int32_t fd)
    {
        linger so_linger;
        so_linger.l_onoff = 0;
        return setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
    }
    /*
    这个套接字选项通知内核，如果端口忙，但TCP状态位于 TIME_WAIT ，可以重用端口。如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息，指明"地址已经使用中"。
    如果你的服务程序停止后想立即重启，而新套接字依旧使用同一端口，此时SO_REUSEADDR 选项非常有用
    */
    int32_t TcpSocket::SetReuseAddrOn(int32_t fd)
    {
        int32_t reuse_addr = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr, sizeof(reuse_addr));
    }

    int32_t TcpSocket::SetDeferAccept(int32_t fd)
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // TODO
        return 0;
#elif defined(__linux__)
        int32_t secs = 1;
        return setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &secs, sizeof(secs));
#elif defined(__APPLE__)
        return 0;
#endif
    }

    int32_t TcpSocket::SetTcpBuffSize(int32_t fd)
    {
        if (send_buff_len_ > static_cast<int32_t>(DEFAULT_CONN_BUFFER_SIZE))
        {
            int32_t snd_size = DEFAULT_CONN_BUFFER_SIZE;
            socklen_t optlen = sizeof(snd_size);
            setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&snd_size, optlen);
        }
        if (recv_buff_len_ > static_cast<int32_t>(DEFAULT_CONN_BUFFER_SIZE))
        {
            int32_t rcv_size = DEFAULT_CONN_BUFFER_SIZE;
            socklen_t optlen = sizeof(rcv_size);
            setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&rcv_size, optlen);
        }
        return 0;
    }

    int32_t TcpSocket::GetSysErrNo()
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return GetLastError();
#elif defined(__linux__) || defined(__APPLE__)
        return errno;
#endif
    }

};  // ToolBox

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