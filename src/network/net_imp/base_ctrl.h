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
    * 在 IO多路复用上 添加/删除 socket 
    */
    virtual bool OperEvent(BaseSocket &socket, EventOperType op_type, int32_t event_type) = 0;
    /*
    * 在IO多路复用删除 socket
    */
    virtual bool DelEvent(int socket_fd){ return true;};
    /*
    * 执行一次. e.g. epollwait;GetQueuedCompletionStatus;kevent.
    */
    virtual bool RunOnce() = 0;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    /*
    * @brief 建立 socket 与 iocp 的关联
    */
    virtual bool AssociateSocketToIocp(BaseSocket& socket) { return true; };
#endif 
};
