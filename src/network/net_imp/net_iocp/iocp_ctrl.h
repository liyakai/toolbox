#pragma once

#include "src/network/net_imp/net_imp_define.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
/*
* ������� iocp ����
*/
class IocpCtrl
{
public:
    /*
    * ����
    * @param max_events ����¼�����
    */
    IocpCtrl();
    /*
    * ���� iocp
    * @return �Ƿ�ɹ�
    */
    bool CreateIocp();
    /*
    * ���� iocp
    */
    void Destroy();
    /*
    * �����¼�
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
    * @brief ���� socket �� iocp �Ĺ���
    */
    template<typename SocketType>
    bool AddSocketToIocp(SocketType& socket)
    {
        // ���� socket �� iocp �Ĺ���
        HANDLE iocp = CreateIoCompletionPort((HANDLE)socket.GetSocketID(), iocp_fd_, (ULONG_PTR)&socket, 0);
        if (nullptr == iocp)
        {
            return false;
        }
        return true;
    }
    /*
    * @brief ���������Ϣ
    */
    template<typename SocketType>
    bool OnRecv(SocketType& socket)
    {
        if (EIOSocketState::IOCP_ACCEPT == socket.GetSocketState())
        {
            // ���� socket �� iocp �Ĺ���
            if (!AddSocketToIocp(socket))
            {
                return false;
            }
            auto& socket_accept_ex = socket.GetAcceptEx();
            if (nullptr == socket_accept_ex)
            {
                // ��ȡ AcceptExָ��
                DWORD bytes = 0;
                GUID guid_accept_ex = WSAID_ACCEPTEX;
                int32_t error_code = WSAIoctl(socket.GetSocketID(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_accept_ex, sizeof(guid_accept_ex),
                    &socket_accept_ex, sizeof(ACCEPTEX), &bytes, nullptr, nullptr);
                if (error_code|| nullptr == socket_accept_ex)
                {
                    return false;
                }
            }
            // ����һ��֧���ص�I/O���׽���
            auto socket_id = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == socket_id)
            {
                return false;
            }
            socket.SetAcceptExSocketId(socket_id);
            // MSDN: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-acceptex
            // Ͷ��һ�� accept ����
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
            // Ͷ��һ������Ϊ0������
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
    * @brief  ��������Ϣ
    */
    template<typename SocketType>
    bool OnSend(SocketType& socket)
    {
        if (EIOSocketState::IOCP_ACCEPT == socket.GetSocketState())
        {
            // ���� socket �� iocp �Ĺ���
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
    * ִ��һ�� iocp
    */
    template<typename SocketType>
    bool RunOnce()
    {
        time_t      time_stamp = time(0);    // ʱ���
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
    HANDLE iocp_fd_;            // iocp �ļ�������
};

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)