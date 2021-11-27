#pragma once

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx ����ָ��,���MSDN

// MSDN: The number of bytes reserved for the local address information. 
// This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
constexpr std::size_t ACCEPTEX_ADDR_SIZE = sizeof(sockaddr_in) + 16;
constexpr std::size_t ACCEPTEX_BUFF_SIZE = 2014;



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

/*
* IOCP��socket��״̬
*/
enum EIOSocketState
{
    IOCP_ACCEPT = 1,    // ����
    IOCP_CONNECT = 2,    // ���ӽ���
    IOCP_RECV = 4,    // ����
    IOCP_SEND = 8,    // ����
    IOCP_CLOSE = 16,   // �ر�
};


/*
* ���� per-I/O ����
*/
struct PerIOContext
{
    OVERLAPPED      over_lapped;    // windows �ص�I/O���ݽṹ
    WSABUF          wsa_buf;        // �洢���ݵĻ�����,�������ص��������ݲ���.
    char            buffer[DEFAULT_CONN_BUFFER_SIZE];   // ��Ӧ WSABUF ��Ļ�����
    EIOSocketState  io_type;        // ��ǰ��I/O����. IOCPû����epoll������EPOLLIN,EPOLLOUT,ֻ��ͨ������Я�������������.
};

/*
* ���� AcceptEx �������
*/
struct AcceptEx_t
{
    ACCEPTEX accept_ex_fn = nullptr;      // AcceptEx ����ָ��
    SOCKET   socket_fd;         // ��ǰδ���Ŀͻ����׽���  -AcceptEx
    char     buffer[ACCEPTEX_BUFF_SIZE];           // ���� AcceptEx
};

/*
* ���� per-socket ����
*/
struct PerSockContext
{
    AcceptEx_t* accept_ex = nullptr;      // AcceptEx_tָ��
    PerIOContext   io_recv;     // ��������
    PerIOContext   io_send;     // ��������
};
#endif
