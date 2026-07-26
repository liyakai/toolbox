#include "base_socket.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <ioapiset.h>
#endif

namespace ToolBox
{

    BaseSocket::BaseSocket()
    {

    }

    BaseSocket::~BaseSocket()
    {
        Reset();
    }
    void BaseSocket::Reset()
    {
        // 这里必须用非虚的 CloseSocketFd:Reset 会在 Free 与析构中执行,
        // 若派发到子类 Close 会再次通知主线程并 Free(this)
        CloseSocketFd();
        opaque_ = 0;
        conn_id_ = INVALID_CONN_ID;
        ++dispatch_generation_;  // 池复用同 index 时令牌必须失效
        ClearInitError();
        // 状态与所有权复位必须在 CloseSocketFd 之后:后者依赖 fd_ownership_ 决定是否真关 fd
        life_ = SockLife::IDLE;
        fd_ownership_ = FdOwnership::OWN;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#elif defined(__linux__)
        is_ctrl_add_ = false;
#endif
        event_type_ = SOCKET_EVENT_INVALID;
    }
    bool BaseSocket::IsSocketValid()
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return INVALID_SOCKET != socket_id_;
#else
        return socket_id_ >= 0;
#endif
    }

    void BaseSocket::Close(ENetErrCode net_err, int32_t sys_err)
    {
        CloseSocketFd();
    }

    void BaseSocket::CloseSocketFd()
    {
        // 先置无效再收尾,借用的 fd 只放弃引用,避免与所有者形成双关
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        const SOCKET fd = socket_id_;
        if (INVALID_SOCKET == fd)
        {
            return;
        }
        socket_id_ = INVALID_SOCKET;
        if (FdOwnership::BORROW == fd_ownership_)
        {
            return;
        }
        CancelIo((HANDLE)fd);
        shutdown(fd, 2);
        closesocket(fd);
#else
        const int32_t fd = socket_id_;
        if (fd < 0)
        {
            return;
        }
        socket_id_ = -1;
        if (FdOwnership::BORROW == fd_ownership_)
        {
            return;
        }
        shutdown(fd, SHUT_RDWR);
        close(fd);
#endif
    }

    int32_t BaseSocket::GetSocketError()
    {
        int32_t error = 0;
#if defined(__linux__) || defined(__APPLE__)
        socklen_t len = sizeof(error);
        if (getsockopt(socket_id_, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
        {
            return 0;
        }
#endif
        return error;
    }

    void BaseSocket::OnErrored(ENetErrCode err_code, int32_t err_no)
    {
        p_network_->OnErrored(GetOpaque(), GetConnID(), err_code, err_no);
    }

};  // ToolBox
