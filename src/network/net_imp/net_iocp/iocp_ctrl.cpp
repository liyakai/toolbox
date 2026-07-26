#include "iocp_ctrl.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "network/net_imp/net_imp_define.h"

namespace ToolBox
{

    IocpCtrl::IocpCtrl()
    {

    }

    bool IocpCtrl::CreateIOMultiplexing()
    {
        SetLastSystemError(0);
        iocp_fd_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (!iocp_fd_)
        {
            SetLastSystemError(static_cast<int32_t>(GetLastError()));
            return false;
        }
        WSADATA wsa;
        int32_t result = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (0 != result)
        {
            SetLastSystemError(result);
            CloseHandle(iocp_fd_);
            iocp_fd_ = nullptr;
            return false;
        }
        return true;
    }

    void IocpCtrl::DestroyIOMultiplexing()
    {
        if (nullptr != iocp_fd_)
        {
            CloseHandle(iocp_fd_);
            iocp_fd_ = nullptr;
            WSACleanup();
        }
    }

    bool IocpCtrl::OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type)
    {
        SetLastSystemError(0);
        if (EventOperType::EVENT_OPER_ADD == op_type)
        {
            // Init 只配 fd;关联放在这里.已关联时 CreateIoCompletionPort 忽略 CompletionKey,可重复调
            if (!AssociateSocketToIocp(socket))
            {
                return false;
            }
            if (SOCKET_EVENT_RECV & event_type)
            {
                return OnRecv(socket);
            }
            else if (SOCKET_EVENT_SEND & event_type)
            {
                return OnSend(socket);
            }
        }
        else if (EventOperType::EVENT_OPER_RDC == op_type)
        {
            auto now_event_type = socket.GetEventType();
            if (SOCKET_EVENT_RECV & event_type)
            {
                now_event_type &= ~SOCKET_EVENT_RECV;
            }
            else if (SOCKET_EVENT_SEND & event_type)
            {
                now_event_type &= ~SOCKET_EVENT_SEND;
            }
            socket.SetSockEventType(now_event_type);
        }
        return true;
    }

    bool IocpCtrl::AssociateSocketToIocp(BaseSocket& socket)
    {
        SetLastSystemError(0);
        auto socket_id = socket.GetSocketID();
        HANDLE iocp = CreateIoCompletionPort((HANDLE)socket_id, iocp_fd_,
                                             static_cast<ULONG_PTR>(socket.GetDispatchToken()), 0);
        if (nullptr == iocp)
        {
            DWORD last_error = GetLastError();
            if (ERROR_IO_PENDING != last_error)
            {
                SetLastSystemError(static_cast<int32_t>(last_error));
                return false;
            }
        }
        return true;
    }

    bool IocpCtrl::OnRecv(BaseSocket& socket)
    {
        socket.ResetRecvAsyncSocket();
        auto* per_socket = socket.GetPerSocket();
        if (SocketState::SOCK_STATE_LISTENING == socket.GetSocketState())
        {
            auto& accept_ex = per_socket->accept_ex;
            if (nullptr == accept_ex)
            {
                accept_ex = new AcceptEx_t;
            }
            if (nullptr == accept_ex->accept_ex_fn)
            {
                // 获取 AcceptEx指针
                DWORD bytes = 0;
                GUID guid_accept_ex = WSAID_ACCEPTEX;
                int32_t error_code = WSAIoctl(socket.GetSocketID(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_accept_ex, sizeof(guid_accept_ex),
                                              &accept_ex->accept_ex_fn, sizeof(ACCEPTEX), &bytes, nullptr, nullptr);
                if (error_code || nullptr == accept_ex->accept_ex_fn)
                {
                    SetLastSystemError(WSAGetLastError());
                    return false;
                }
            }
            // 建立一个支持重叠I/O的套接字
            accept_ex->socket_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == accept_ex->socket_fd)
            {
                SetLastSystemError(WSAGetLastError());
                return false;
            }
            // MSDN: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-acceptex
            // 投递一个 accept 请求
            per_socket->io_recv.io_type = SocketState::SOCK_STATE_LISTENING;
            bool result = accept_ex->accept_ex_fn(socket.GetSocketID(), accept_ex->socket_fd, accept_ex->buffer, 0,
                                                  ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, 0, &per_socket->io_recv.over_lapped);
            if (false == result)
            {
                DWORD last_error = WSAGetLastError();
                // MSDN:  If WSAGetLastError returns ERROR_IO_PENDING,
                // then the operation was successfully initiated and is still in progress.
                if (ERROR_IO_PENDING != last_error)
                {
                    SetLastSystemError(static_cast<int32_t>(last_error));
                    closesocket(accept_ex->socket_fd);
                    accept_ex->socket_fd = INVALID_SOCKET;
                    return false;
                }
            }
            return true;
        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            // 投递一个长度为0的请求
            DWORD bytes = 0;
            DWORD flags = 0;
            auto& io_recv = per_socket->io_recv;
            per_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
            int32_t result = WSARecv(socket.GetSocketID(), &io_recv.wsa_buf, 1, &bytes, &flags, &io_recv.over_lapped, 0);
            // result 为 0 表示立即完成,同样会投递完成通知,与 IO_PENDING 一样视为成功
            if (0 != result)
            {
                uint64_t error = WSAGetLastError();
                if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
                {
                    NetworkLogError("[Network] WSARecv failed. fd:%llu, error:%llu",
                                    (uint64_t)socket.GetSocketID(), error);
                    SetLastSystemError(static_cast<int32_t>(error));
                    return false;
                }
            }
            return true;
        }
        // 其余状态无需在此投递接收请求,不算失败
        return true;
    }
    /*
    * @brief  处理发送消息
    */
    bool IocpCtrl::OnSend(BaseSocket& socket)
    {
        socket.ResetSendAsyncSocket();
        auto& io_send = socket.GetPerSocket()->io_send;
        if (SocketState::SOCK_STATE_CONNECTING == socket.GetSocketState())
        {
            io_send.io_type = SocketState::SOCK_STATE_CONNECTING;
        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            io_send.io_type = SocketState::SOCK_STATE_SEND;
        }
        // 投递一个send请求
        DWORD bytes = 0;
        DWORD flags = 0;
        int32_t result = WSASend(socket.GetSocketID(), &io_send.wsa_buf, 1, &bytes, flags, &io_send.over_lapped, 0);
        // result 为 0 表示立即完成,同样会投递完成通知,与 IO_PENDING 一样视为成功
        if (0 != result)
        {
            uint64_t error = WSAGetLastError();
            if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
            {
                NetworkLogError("[Network] WSASend failed. fd:%llu, error:%llu",
                                (uint64_t)socket.GetSocketID(), error);
                SetLastSystemError(static_cast<int32_t>(error));
                return false;
            }
        }
        return true;
    }

    bool IocpCtrl::RunOnce(std::time_t time_stamp)
    {
        DWORD       bytes = 0;
        ULONG_PTR   completion_key = 0;
        PerIOContext* per_io = nullptr;
        bool error = GetQueuedCompletionStatus(iocp_fd_, &bytes, &completion_key, (LPOVERLAPPED*)&per_io, 2);
        auto last_error = GetLastError();
        if (false == error)
        {
            if (WAIT_TIMEOUT == last_error || ERROR_OPERATION_ABORTED == last_error)
            {
                return true;
            }
            SetLastSystemError(static_cast<int32_t>(last_error));
            BaseSocket* socket = ResolveSockDispatchToken(static_cast<uint64_t>(completion_key));
            if (nullptr != socket)
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                return true;
            }
            return false;
        }
        if (0 == completion_key || nullptr == per_io)
        {
            SetLastSystemError(ERROR_INVALID_DATA);
            return false;
        }
        BaseSocket* socket = ResolveSockDispatchToken(static_cast<uint64_t>(completion_key));
        if (nullptr == socket)
        {
            // 令牌过期:连接已 Close,丢弃迟到完成包
            return true;
        }
        per_io->wsa_buf.len = bytes;
        if ((per_io->io_type & SocketState::SOCK_STATE_RECV) || (per_io->io_type & SocketState::SOCK_STATE_LISTENING))
        {
            socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        }
        else if ((per_io->io_type & SocketState::SOCK_STATE_SEND) || (per_io->io_type & SocketState::SOCK_STATE_CONNECTING))
        {
            socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        }

        return true;
    }

};  // ToolBox

#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
