#pragma once
#include <stdint.h>
#include "epoll_define.h"
/*
* ����ÿһ������
*/
class EpollSocket
{
public:
    /*
    * ����
    */
    EpollSocket();
    /*
    * ����
    */
    ~EpollSocket();
    /*
    * ��ʼ��
    */
    bool Init(SocketType type, uint32_t send_buff_len, uint32_t  recv_buff_len);
    /*
    * ���ʼ��
    */
    void UnInit();
    /*
    * ����
    */
    void Reset();
    /*
    * ��ȡ�¼�����
    * @params ��Ͷ���¼�����
    */
    SockEventType GetEventType() const;

private:
    uint32_t id_ = 0;   // socket_id
    uint32_t ip_ = 0;
    uint16_t port_ = 0;

    EpollData epoll_data_;

    SockEventType event_type_;  // ��Ͷ���¼�����
};