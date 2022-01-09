#include "base_socket.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <ioapiset.h>
#endif

namespace ToolBox{

BaseSocket::BaseSocket()
{

}

BaseSocket::~BaseSocket()
{
    Reset();
}
void BaseSocket::Reset()
{
    Close(ENetErrCode::NET_NO_ERROR, 0);
    conn_id_ = INVALID_CONN_ID;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#elif defined(__linux__)
    is_ctrl_add_ = false;
#endif
    event_type_ = SOCKET_EVENT_INVALID;
}
bool BaseSocket::IsSocketValid()
{
    return socket_id_ > 0;
}

void BaseSocket::Close(ENetErrCode net_err, int32_t sys_err)
{
    if (IsSocketValid())
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        CancelIo((HANDLE)socket_id_);
        shutdown(socket_id_, 2);
        closesocket(socket_id_);
#elif defined(__linux__)
        shutdown(socket_id_, SHUT_RDWR);
        close(socket_id_);
#endif // __linux__
        socket_id_ = -1;
    }
}

int32_t BaseSocket::GetSocketError()
{
    int32_t error = 0;
#ifdef __linux__
    socklen_t len = sizeof(error);
    if(getsockopt(socket_id_, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
    {
        return 0;
    }
#endif // __linux__
    return error;
}

void BaseSocket::OnErrored(ENetErrCode err_code, int32_t err_no)
{
    p_network_->OnErrored(GetConnID(), err_code, err_no);
}

};  // ToolBox
