#pragma once

#include "network.h"
#include "net_epoll/epoll_ctrl.h"

/*
* ���� Tcp����
*/
class TcpNetwork : public INetwork
{
public:
    /*
    * ����
    */
    TcpNetwork();
    /*
    * ����
    */
    virtual ~TcpNetwork();
    /*
    * ��ʼ��
    */
    virtual void Init(NetworkMaster* master) override;
    /*
    * ���ʼ������
    */
    virtual void UnInit() override;
    /*
    * ִ��һ������ѭ��
    */
    virtual void Update() override;
   
protected:
    /*
    * �����߳��ڽ���������
    */
    virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port) override;
    /*
    * �����߳��ڽ���������
    */
    virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port) override;
    /*
    * �ع����߳��ڱ���������
    */
    virtual void OnClose(uint64_t connect_id) override;
    /*
    * �����߳��ڹ����߳��ڷ���
    */
    virtual void OnSend(uint64_t connect_id, const char* data, uint32_t size) override;

private:
    NetworkMaster *network_master_;
    EpollCtrl epoll_ctrl_;
};
