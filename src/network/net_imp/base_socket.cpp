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
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        if (INVALID_SOCKET == socket_id_)
        {
            return;
        }
        CancelIo((HANDLE)socket_id_);
        shutdown(socket_id_, 2);
        closesocket(socket_id_);
        socket_id_ = INVALID_SOCKET;
#else
        if (socket_id_ < 0)
        {
            return;
        }
        shutdown(socket_id_, SHUT_RDWR);
        close(socket_id_);
        socket_id_ = -1;
#endif
    }

    int32_t BaseSocket::GetSocketError()
    {
        int32_t error = 0;
#ifdef __linux__
        socklen_t len = sizeof(error);
        if (getsockopt(socket_id_, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
        {
            return 0;
        }
#endif // __linux__
        return error;
    }

    void BaseSocket::OnErrored(ENetErrCode err_code, int32_t err_no)
    {
        p_network_->OnErrored(GetOpaque(), GetConnID(), err_code, err_no);
    }

};  // ToolBox
