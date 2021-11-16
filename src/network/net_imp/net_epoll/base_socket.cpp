#include "base_socket.h"

BaseSocket::BaseSocket()
{

}

BaseSocket::~BaseSocket()
{
    Reset();
}
void BaseSocket::Reset()
{
    Close();
    conn_id_ = INVALID_CONN_ID;
    is_ctrl_add_ = false;
    event_type_ = SOCKET_EVENT_INVALID;
}
bool BaseSocket::IsSocketValid()
{
    return socket_id_ > 0;
}

void BaseSocket::Close()
{
    if (IsSocketValid())
    {
#ifdef __linux__
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