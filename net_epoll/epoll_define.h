#pragma once

#define MAX_SOCKET_COUNT 20000
#define MAXEVENTS 8192 /* epoll_create���� */
#define EPOLL_WAIT_MSECONDS 2

/*
* ������
*/
enum class ErrCode
{
    ERR_SUCCESS = 0,
    ERR_MALLOC_FAILED, // �����ڴ�ʧ��
    ERR_PARAM_NULLPTR, // ����Ϊ��ָ��

};

enum class SocketType
{
    SOCK_TYPE_INVALID = 0,
    SOCK_TYPE_CONNECT,
    SOCK_TYPE_LISTEN,
};

enum class SocketState
{
    SOCK_STATE_INVALIE = 0,
    SOCK_STATE_LISTENING,
    SOCK_STATE_CONNECTING,
    SOCK_STATE_ESTABLISHED,
};

/*
* epoll ����
*/
enum class EpollOperType
{
    EPOLL_OPER_ADD = 1,
    EPOLL_OPER_RDC = 2,
};

/*
* socket ��Ͷ���¼�
*/
enum SockEventType
{
    SOCKET_EVENT_RECV = 1,
    SOCKET_EVENT_SEND = 2,
};

struct EpollData
{
    SocketType socket_type; // socket type
    void *ptr;              // socket ptr
    EpollData()
        : socket_type(SocketType::SOCK_TYPE_INVALID), ptr(nullptr)
    {
    }
    void Reset()
    {
        socket_type = SocketType::SOCK_TYPE_INVALID;
        ptr = nullptr;
    }
};