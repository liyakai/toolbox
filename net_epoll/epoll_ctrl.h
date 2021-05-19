#pragma once

#include "tools/singleton.h"
#include <sys/epoll.h>
/*
* epoll 类
*/
class EpollCtrl
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    EpollCtrl(uint max_events);
    /*
    * 创建 epoll
    * @return 错误码
    */
    bool CreateEpoll();
    /*
    * 销毁 epoll
    */
    void Destroy();
    /*
    * 添加事件
    * @param socket_fd 文件描述符
    * @param event  事件类型
    * @param ptr 透传指针
    * @return 是否成功
    */
    bool AddEvent(int socket_fd, int event, void* ptr);
    /*
    * 修改事件
    * @param socket_fd 文件描述符
    * @param event  事件类型
    * @param ptr 透传指针
    * @return 是否成功
    */
    bool ModEvent(int socket_fd, int event, void* ptr);
    /*
    * 删除事件
    * @param socket_fd 文件描述符
    */
    bool DelEvent(int socket_fd);
    /*
    * epoll_wait
    * @param msec 等待毫秒数
    * @return 有读写通知的文件描述符的个数
    */
    int EpollWait(int msec);
    /*
    * 获取事件
    * @param index 事件序号
    * @return epoll_event*
    */
    epoll_event* GetEvent(int index);
private:

private:
    uint32_t max_events_ = 0;   // 最大事件数
    int      epoll_fd_;         // epoll 文件描述符
    epoll_event* events_;       // epoll 事件数组
};

#define EpollCtrlMgr Singleton<EpollCtrl>::Instance(10240)