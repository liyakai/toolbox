#pragma once

#include "network.h"


/*
* 定义 Tcp网络
*/
class TcpNetwork : public INetwork
{
public:
    /*
    * 构造
    */
    TcpNetwork();
    /*
    * 析构
    */
    virtual ~TcpNetwork();
    /*
    * 初始化
    */
    virtual void Init(NetworkMaster* master) override;
    /*
    * 执行一次网络循环
    */
    virtual void Update() override;
   

private:

}
