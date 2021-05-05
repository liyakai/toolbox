#pragma once

#include "tools/singleton.h"
#include "epoll_define.h"
#include <sys/epoll.h>
/*
* epoll 类
*/
class NetEpoll
{
public:
    /*
    * 构造
    * @param max_events 最大事件数量
    */
    NetEpoll(uint32_t max_events);
    /*
    * 创建 epoll
    * @return 错误码
    */
    ErrCode CreateEpoll();
    /*
    * 销毁 epoll
    */
    void Destroy();
    /*
    * 添加读写事件
    */
    ErrCode AddEvent(int op, int socket_fd);
private:
    /*
    * epoll_wait
    * @param *count 输出 event 的个数
    & @return 错误码
    */
    ErrCode EpollWait(int32_t *count);
private:
    uint32_t max_events_ = 0;   // 最大事件数
    int      epoll_fd_;         // epoll 文件描述符
    epoll_event* events_;       // epoll 事件数组
};

#define NetEpollMgr Singleton<NetEpoll>::instance(10240)