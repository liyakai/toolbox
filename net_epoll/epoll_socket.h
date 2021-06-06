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
    bool Init(SocketType type, uint32_t send_buff_len, uint32_t recv_buff_len);
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
    * @return ��Ͷ���¼�����
    */
    SockEventType GetEventType() const;
    /*
    * ��ȡ�ļ�������
    * @return �ļ�������
    */
    uint32_t GetSocketID() { return id_; }
    /*
    * ��ȡ socket �Ƿ�ִ�й� EPOLL_CTL_ADD
    * @return �Ƿ�ִ�й� EPOLL_CTL_ADD
    */
    bool IsCtrlAdd() { return is_ctrl_add_; }
    /*
    * ��ȡ socket �Ƿ�ִ�й� EPOLL_CTL_ADD
    * @params �Ƿ�ִ�й� EPOLL_CTL_ADD
    */
    void SetCtrlAdd(bool value);

private:
    uint32_t id_ = 0; // socket_id
    uint32_t ip_ = 0;
    uint16_t port_ = 0;

    EpollData epoll_data_;

    SockEventType event_type_; // ��Ͷ���¼�����
    bool is_ctrl_add_ = false; // �Ƿ��Ѿ�ִ�й� EPOLL_CTL_ADD
};