#pragma once

#include "src/network/net_imp/net_imp_define.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

enum EIOType
{
    IOT_ACCEPT  = 1,    // ������
    IOT_CONNECT = 2,    // ���ӽ���
    IOT_RECV    = 4,    // ����
    IOT_SEND    = 8,    // ����
    IOT_CLOSE   = 16,   // �ر�
};

/*
* ���� per-I/O ����
*/
struct PerIO_t
{
    OVERLAPPED over_lapped; // windows �ص�I/O���ݽṹ
    EIOType    io_type;     // ��ǰ��I/O����
};

/*
* ���� AcceptEx �������
*/
struct AcceptEx_t
{
    ACCEPTEX accept_ex_fn;      // AcceptEx ����ָ��
    SOCKET   socket_fd;         // ��ǰδ���Ŀͻ����׽���  -AcceptEx
    char     buffer[DEFAULT_CONN_BUFFER_SIZE];           // ���� AcceptEx
};

/*
* ���� per-socket ����
*/
template<typename SocketType>
struct PerSock_t
{
    SocketType* net_socket;     // socket ָ��
    AcceptEx_t* accept_ex_info; // AcceptEx_tָ��
    PerIO_t   io_recv;
    PerIO_t   io_send;
};

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
        if ((per_io->io_type & EIOType::IOT_RECV) || (per_io->io_type & EIOType::IOT_ACCEPT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
        } else if ((per_io->io_type & EIOType::IOT_SEND) || (per_io->io_type & EIOType::IOT_CONNECT))
        {
            per_sock->net_socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
        }

        return true;
    }
private:
    HANDLE iocp_fd_;            // iocp �ļ�������
};

#endif  // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)