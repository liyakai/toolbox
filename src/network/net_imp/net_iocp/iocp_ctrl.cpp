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
        iocp_fd_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (!iocp_fd_)
        {
            return false;
        }
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        return true;
    }

    void IocpCtrl::DestroyIOMultiplexing()
    {
        CloseHandle(iocp_fd_);
        WSACleanup();
    }

    bool IocpCtrl::OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type)
    {
        if (EventOperType::EVENT_OPER_ADD == op_type)
        {
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
                    return false;
                }
            }
            // 建立一个支持重叠I/O的套接字
            accept_ex->socket_fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == accept_ex->socket_fd)
            {
                return false;
            }
            // MSDN: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-acceptex
            // 投递一个 accept 请求
            per_socket->io_recv.io_type = SocketState::SOCK_STATE_LISTENING;
            bool result = accept_ex->accept_ex_fn(socket.GetSocketID(), accept_ex->socket_fd, accept_ex->buffer, 0,
                                                  ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, 0, &per_socket->io_recv.over_lapped);
            if (false == result)
            {
                DWORD last_error = GetLastError();
                // MSDN:  If WSAGetLastError returns ERROR_IO_PENDING,
                // then the operation was successfully initiated and is still in progress.
                if (ERROR_IO_PENDING != last_error)
                {
                    socket.OnErrored(ENetErrCode::NET_SYS_ERROR, last_error);
                }
            }

        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            // 投递一个长度为0的请求
            DWORD bytes = 0;
            DWORD flags = 0;
            auto& io_recv = per_socket->io_recv;
            per_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
            int32_t result = WSARecv(socket.GetSocketID(), &io_recv.wsa_buf, 1, &bytes, &flags, &io_recv.over_lapped, 0);
            if (result != 0)
            {
                uint64_t error = GetLastError();
                if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
                {
                    return false;
                }
                return true;
            }
        }
        return false;
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
        if (result != 0)
        {
            uint64_t error = GetLastError();
            if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    bool IocpCtrl::RunOnce(std::time_t time_stamp)
    {
        DWORD       bytes = 0;
        BaseSocket* socket = nullptr;
        PerIOContext* per_io = nullptr;
        bool error = GetQueuedCompletionStatus(iocp_fd_, &bytes, (PULONG_PTR)&socket, (LPOVERLAPPED*)&per_io, 2);
        auto last_error = GetLastError();
        if (false == error)
        {
            if (WAIT_TIMEOUT == last_error || ERROR_OPERATION_ABORTED == last_error)
            {
                return true;
            }
            else
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                return false;
            }
        }
        //if ((0 == bytes) && (IOCP_RECV == per_io->io_type || IOCP_SEND == per_io->io_type))
        //{
        //    socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
        //    return false;
        //}
        if (nullptr == socket || nullptr == per_io)
        {
            return false;
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
