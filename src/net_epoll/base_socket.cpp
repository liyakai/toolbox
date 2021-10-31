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
        close(socket_id_);
        socket_id_ = -1;
    }
}