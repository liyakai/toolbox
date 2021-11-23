#pragma once

#include "src/network/net_imp/net_imp_define.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
/*
* 定义管理 iocp 的类
*/
class IocpCtrl
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    IocpCtrl();
    /*
    * 创建 iocp
    * @return 是否成功
    */
    bool CreateIocp();
    /*
    * 销毁 iocp
    */
    void Destroy();
    /*
    * 处理事件
    */
    template<typename SocketType>
    bool OperEvent(SocketType& socket, EventOperType op_type, int32_t event_type)
    {
        if (EventOperType::EVENT_OPER_ADD == op_type)
        {
            if (SOCKET_EVENT_RECV & event_type)
            {
                return OnRecv(socket);
            } else if (SOCKET_EVENT_SEND & event_type)
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
    /*
    * @brief 建立 socket 与 iocp 的关联
    */
    template<typename SocketType>
    bool AddSocketToIocp(SocketType& socket)
    {
        // 建立 socket 与 iocp 的关联
        HANDLE iocp = CreateIoCompletionPort((HANDLE)socket.GetSocketID(), iocp_fd_, (ULONG_PTR)&socket, 0);
        if (nullptr == iocp)
        {
            return false;
        }
        return true;
    }
    /*
    * @brief 处理接收消息
    */
    template<typename SocketType>
    bool OnRecv(SocketType& socket)
    {
        if (EIOSocketState::IOCP_ACCEPT == socket.GetSocketState())
        {
            // 建立 socket 与 iocp 的关联
            if (!AddSocketToIocp(socket))
            {
                return false;
            }
            auto& socket_accept_ex = socket.GetAcceptEx();
            if (nullptr == socket_accept_ex)
            {
                // 获取 AcceptEx指针
                DWORD bytes = 0;
                GUID guid_accept_ex = WSAID_ACCEPTEX;
                int32_t error_code = WSAIoctl(socket.GetSocketID(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_accept_ex, sizeof(guid_accept_ex),
                    &socket_accept_ex, sizeof(ACCEPTEX), &bytes, nullptr, nullptr);
                if (error_code|| nullptr == socket_accept_ex)
                {
                    return false;
                }
            }
            // 建立一个支持重叠I/O的套接字
            auto socket_id = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == socket_id)
            {
                return false;
            }
            socket.SetAcceptExSocketId(socket_id);
            // MSDN: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-acceptex
            // 投递一个 accept 请求
            bool result = socket_accept_ex(socket.GetSocketID(), socket_id, socket.GetBuffer(), 0,
                            ACCEPTEX_ADDR_SIZE, ACCEPTEX_ADDR_SIZE, 0, &socket.GetOverLapped());
            if (false == result)
            {
                DWORD last_error = GetLastError();
                // MSDN:  If WSAGetLastError returns ERROR_IO_PENDING, 
                // then the operation was successfully initiated and is still in progress.
                if (ERROR_IO_PENDING != last_error)
                {
                    Destroy();
                }
            }

        }
        else if (EIOSocketState::IOCP_RECV == socket.GetSocketState())
        {
            // 投递一个长度为0的请求
            WSABUF  sbuff = { 0, nullptr };
            DWORD bytes = 0;
            DWORD flags = 0;
            int32_t result = WSARecv(socket.GetSocketID(), &sbuff, 1, &bytes, &flags, socket.GetOverLappedPtr(), 0);
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
    }
    /*
    * @brief  处理发送消息
    */
    template<typename SocketType>
    bool OnSend(SocketType& socket)
    {
        if (EIOSocketState::IOCP_ACCEPT == socket.GetSocketState())
        {
            // 建立 socket 与 iocp 的关联
            if (!AddSocketToIocp(socket))
            {
                return false;
            }
        }
        else if (EIOSocketState::IOCP_SEND == socket.GetSocketState())
        {

        }
        return true;
    }
    /*
    * 执行一次 iocp
    */
    template<typename SocketType>
    bool RunOnce()
    {
        time_t      time_stamp = time(0);    // 时间戳
        DWORD       bytes = 0;
        PerSock_t<SocketType>* per_sock = nullptr;
        PerIO_t* per_io = nullptr;
        bool error = GetQueuedCompletionStatus(iocp_fd_, &bytes, (PULONG_PTR)&per_sock, (LPOVERLAPPED*)&per_io, 2);
        auto last_error = GetLastError();
        if (false == error)
        {
            if (WAIT_TIMEOUT == last_error || ERROR_OPERATION_ABORTED == last_error)
            {
                return true;
            }
        }
        if (nullptr == per_sock || nullptr == per_io || nullptr == per_sock->net_socket)
        {
            return false;
        }
        if ((per_io->io_type & EIOSocketState::IOCP_RECV) || (per_io->io_type & EIOSocketState::IOCP_ACCEPT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        } else if ((per_io->io_type & EIOSocketState::IOCP_SEND) || (per_io->io_type & EIOSocketState::IOCP_CONNECT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        }

        return true;
    }
private:
    HANDLE iocp_fd_;            // iocp 文件描述符
};

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)