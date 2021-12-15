#pragma once
#include "net_imp_define.h"
#include "base_socket.h"
/*
* @brief IOMultiplexingInterface 是io多路复用的接口.
*/
class IOMultiplexingInterface
{
public:
    /*
    * 基类虚析构
    */
    virtual ~IOMultiplexingInterface(){};
    /*
    * 创建IO多路复用
    * @return 是否成功
    */
    virtual bool CreateIOMultiplexing() = 0;
    /*
    * 销毁IO多路复用
    */
    virtual void DestroyIOMultiplexing() = 0;
    /*
    * 添加/删除IO多路复用
    */
    virtual bool OperEvent(BaseSocket &socket, EventOperType op_type, int32_t event_type) = 0;
    /*
    * 执行一次. e.g. epollwait;GetQueuedCompletionStatus;kevent.
    */
    virtual bool RunOnce() = 0;
};
