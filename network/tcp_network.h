#pragma once

#include "network.h"


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
    * ִ��һ������ѭ��
    */
    virtual void Update() override;
   

private:

}
