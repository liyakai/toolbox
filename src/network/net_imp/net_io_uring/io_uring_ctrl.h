#pragma once

#ifdef LINUX_IO_URING

#include "src/tools/singleton.h"
#include "src/network/net_imp/net_imp_define.h"
#include "src/network/net_imp/base_ctrl.h"
#include "liburing.h"


namespace ToolBox
{

    struct IOUringConnInfo
    {
        uint32_t fd;
        uint16_t type;
        uint16_t bid;
    };

    /*
    * io_uring 控制类
    */
    class IOUringCtrl : public IOMultiplexingInterface
    {
    public:
        /*
        * 构造
        * @param max_events 最大事件数量
        */
        IOUringCtrl(uint32_t max_events);
        /*
        * 析构
        */
        ~IOUringCtrl();
        /*
        * 创建 epoll
        * @return 是否成功
        */
        bool CreateIOMultiplexing() override;
        /*
        * 销毁 epoll
        */
        void DestroyIOMultiplexing() override;
        /*
        * 删除事件
        * @param socket_fd 文件描述符
        */
        // bool DelEvent(int socket_fd);
        /*
        * 处理事件
        */
        bool OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type) override;
        /*
        * epoll_wait
        * @param msec 等待毫秒数
        * @return 有读写通知的文件描述符的个数
        */
        int32_t URingWait(int msec);
        /*
        * 执行一次 epoll wait
        */
        bool RunOnce() override;
    private:
        /*
        * @brief 接收消息
        * @param socket 客户端连接
        * @param flags 标记
        * @return 是否成功
        */
        bool OnRecv(BaseSocket& socket, uint32_t flags);
        /*
        * @brief 发送消息
        * @param socket 客户端连接
        * @param flags 标记
        * @return 是否成功
        */
        bool OnSend(BaseSocket& socket, uint32_t flags);
        /*
        * @brief 监听
        * @param socket 客户端连接
        * @param message_size 消息大小
        * @param flags 标记
        */
        bool AddSocketAccept(BaseSocket* socket, uint32_t flags);
        /*
        * @brief 读
        * @param socket 客户端连接
        * @param message_size 消息大小
        * @param flags 标记
        */
        void AddSocketRead(BaseSocket& socket, std::size_t message_size, uint32_t flags);
        /*
        * @brief 写
        * @param socket 客户端连接
        * @param bid
        * @param message_size 消息大小
        * @param flags 标记
        */
        void AddSocketWrite(BaseSocket& socket, uint16_t bid, std::size_t message_size, uint32_t flags);
        /*
        * @brief 提供buf
        * @param socket 客户端连接
        * @param bid
        * @param message_size 消息大小
        * @param flags 标记
        */
        void AddProvideBuf(BaseSocket& socket, uint16_t bid);
    private:
        uint32_t max_events_ = 0; // 最大事件数
        struct io_uring* ring_;       // io_uring 全局句柄
        char bufs_[BUFFERS_COUNT][MAX_MESSAGE_LEN] = {0};
        int32_t group_id_ = 1337;
    };

#define EpollCtrlMgr Singleton<ToolBox::EpollCtrl>::Instance(10240)



};  // ToolBox

#endif // LINUX_IO_URING
