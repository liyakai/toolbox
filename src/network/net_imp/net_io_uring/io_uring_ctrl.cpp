#include "io_uring_ctrl.h"
#ifdef __linux__

namespace ToolBox{

IOUringCtrl::IOUringCtrl(uint max_events)
    : max_events_(max_events)
{
}

bool IOUringCtrl::CreateIOMultiplexing()
{
    // initialize io_uring
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    if (io_uring_queue_init_params(max_events_, &ring_, &params) < 0) {
        return false;
    }

    // check if IORING_FEAT_FAST_POLL is supported
    if (!(params.features & IORING_FEAT_FAST_POLL)) {
        // printf("IORING_FEAT_FAST_POLL not available in the kernel, quiting...\n");
        return false;
    }

    // check if buffer selection is supported
    struct io_uring_probe *probe;
    probe = io_uring_get_probe_ring(&ring_);
    if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS)) {
        // printf("Buffer select not supported, skipping...\n");
        return false;
    }
    free(probe);

    // register buffers for buffer selection
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    sqe = io_uring_get_sqe(&ring_);
    io_uring_prep_provide_buffers(sqe, bufs_, MAX_MESSAGE_LEN, BUFFERS_COUNT, group_id_, 0);

    io_uring_submit(&ring_);
    io_uring_wait_cqe(&ring_, &cqe);
    if (cqe->res < 0) {
        // printf("cqe->res = %d\n", cqe->res);
        return false;
    }
    io_uring_cqe_seen(&ring_, cqe);

    return true;
}
void IOUringCtrl::DestroyIOMultiplexing()
{

    // if (events_ != nullptr)
    // {
    //     delete[] events_;
    //     events_ = nullptr;
    // }
    // if (-1 != epoll_fd_)
    // {
    //     close(epoll_fd_);
    //     epoll_fd_ = -1;
    // }
}

// bool IOUringCtrl::DelEvent(int socket_fd)
// {
//     epoll_event evt;
//     return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd, &evt) == 0;
// }

bool IOUringCtrl::OperEvent(BaseSocket &socket, EventOperType op_type, int32_t event_type)
{
    if (EventOperType::EVENT_OPER_ADD == op_type)
    {
        if (SOCKET_EVENT_RECV & event_type)
        {
            return OnRecv(socket, 0);
        }
        else if (SOCKET_EVENT_SEND & event_type)
        {
            return OnSend(socket, 0);
        }
    }
    else if (EventOperType::EVENT_OPER_RDC == op_type)
    {
        auto now_event_type = socket.GetEventType();
        if (SOCKET_EVENT_RECV & event_type)
        {
            now_event_type &= ~SOCKET_EVENT_RECV;
        }
        else if (SOCKET_EVENT_SEND & event_type)
        {
            now_event_type &= ~SOCKET_EVENT_SEND;
        }
        socket.SetSockEventType(now_event_type);
    }
    return true;
}

int32_t IOUringCtrl::URingWait(int msec)
{
    return io_uring_submit_and_wait(&ring_, msec);
}

bool IOUringCtrl::RunOnce()
{
    time_t time_stamp = time(0);    // 时间戳
    int32_t count = URingWait(URING_WAIT_MSECONDS);
    if (count < 0)
    {
        return false;
    }
    // io_uring 设置了两个ringbuffer：
    // sq(submission queue):存放提交的IO请求,应用层为生产者操作tail,内核为消费者操作head.其中的entry称为sqe.
    // cq(completion queue):存放处理完成的IO请求,内核为生产者操作tail,应用层为消费者操作head.其中的entry称为cqe.
    struct io_uring_cqe *cqe;
    unsigned head;
    unsigned cqe_count = 0;

    // go through all CQEs
    io_uring_for_each_cqe(&ring_, head, cqe) {
        ++cqe_count;
        UringIOContext* uring_io = reinterpret_cast<UringIOContext*>(cqe->user_data);
        if (nullptr == uring_io) continue;
        BaseSocket* socket = uring_io->base_socket;
        if (nullptr == socket) continue;
        if (cqe->res == -ENOBUFS) {
                // fprintf(stdout, "bufs in automatic buffer selection empty, this should not happen...\n");
                // fflush(stdout);
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                // epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);

                continue;
        }

        if(SOCK_STATE_PROV_BUF & socket->GetEventType())
        {
            if(cqe->res < 0)
            {
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
            }
        } else if (SOCK_STATE_LISTENING & socket->GetEventType())
        {
            int32_t sock_conn_fd = cqe->res;
            // only read when there is no error, >= 0
            if (sock_conn_fd >= 0) {
                AddSocketRead(*socket, MAX_MESSAGE_LEN, IOSQE_BUFFER_SELECT);
            }

            // new connected client; read data from socket and re-add accept to monitor for new connections
            AddSocketAccept(*socket, 0);
        } else if (SOCK_STATE_RECV & socket->GetEventType())
        {
            int32_t bytes_read = cqe->res;
            int32_t bid = cqe->flags >> 16;
            // if (cqe->res <= 0) {
            //     // read failed, re-add the buffer
            //     add_provide_buf(&ring, bid, group_id);
            //     // connection closed or error
            //     close(conn_i.fd);
            // } else {
            //     // bytes have been read into bufs, now add write to socket sqe
            //     add_socket_write(&ring, conn_i.fd, bid, bytes_read, 0);
            // }
        }

    }
    io_uring_cq_advance(&ring_, count);
    // for (int32_t i = 0; i < count; i++)
    // {
    //     epoll_event& event = events_[i];
    //     BaseSocket* socket = static_cast<BaseSocket*>(event.data.ptr);
    //     if (nullptr == socket) continue;
    //     if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
    //     {
    //         socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
    //         epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->GetSocketID(), &evt);
    //     }
    //     if (event.events & EPOLLIN)
    //     {
    //         socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
    //     }
    //     if (event.events & EPOLLOUT)
    //     {
    //         socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
    //     }
    // }
    return true;
}

bool IOUringCtrl::OnRecv(BaseSocket &socket, uint32_t flags)
{
    // socket.ResetRecvPerSocket();
    auto* uring_socket = socket.GetUringSocket();
    if (SocketState::SOCK_STATE_LISTENING == socket.GetSocketState())
    {
        AddSocketAccept(socket, flags);
    }
    else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
    {
        // // 投递一个长度为0的请求
        // DWORD bytes = 0;
        // DWORD flags = 0;
        // auto& io_recv = per_socket->io_recv;
        // per_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
        // int32_t result = WSARecv(socket.GetSocketID(), &io_recv.wsa_buf, 1, &bytes, &flags, &io_recv.over_lapped, 0);
        // if (result != 0)
        // {
        //     uint64_t error = GetLastError();
        //     if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
        //     {
        //         return false;
        //     }
        //     return true;
        // }
    }
    return false;
}

bool IOUringCtrl::OnSend(BaseSocket &socket, uint32_t flags)
{
    // socket.ResetSendPerSocket();
    auto& io_send = socket.GetUringSocket()->io_send;
    if (SocketState::SOCK_STATE_CONNECTING == socket.GetSocketState())
    {
        io_send.io_type = SocketState::SOCK_STATE_CONNECTING;
    }
    else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
    {
        io_send.io_type = SocketState::SOCK_STATE_SEND;
    }
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    // io_uring_prep_send(sqe, socket.GetSocketID(), &bufs_[bid], message_size, 0);
    // io_uring_sqe_set_flags(sqe, flags);
    // // TODO 处理 bid
    // sqe->user_data = reinterpret_cast<uint64_t>(&socket);

    // // 投递一个send请求
    // DWORD bytes = 0;
    // DWORD flags = 0;
    // int32_t result = WSASend(socket.GetSocketID(), &io_send.wsa_buf, 1, &bytes, flags, &io_send.over_lapped, 0);
    // if (result != 0)
    // {
    //     uint64_t error = GetLastError();
    //     if ((ERROR_IO_PENDING != error) && (WSAENOTCONN != error))
    //     {
    //         return false;
    //     }
    //     return true;
    // }
    return false;
}

bool IOUringCtrl::AddSocketAccept(BaseSocket &socket, uint32_t flags)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    io_uring_prep_accept(sqe, socket.GetSocketID(), reinterpret_cast<struct sockaddr*>(&client_addr), &client_len, flags);
    io_uring_sqe_set_flags(sqe, flags);

    auto* uring_socket = socket.GetUringSocket();
    uring_socket->io_recv.base_socket = &socket;
    uring_socket->io_recv.io_type = SocketState::SOCK_STATE_LISTENING;
    sqe->user_data = reinterpret_cast<uint64_t>(uring_socket);
}

void IOUringCtrl::AddSocketRead(BaseSocket &socket, std::size_t message_size, uint32_t flags)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    io_uring_prep_recv(sqe, socket.GetSocketID(), NULL, message_size, 0);
    io_uring_sqe_set_flags(sqe, flags);
    sqe->buf_group = group_id_;

    auto* uring_socket = socket.GetUringSocket();
    uring_socket->io_recv.base_socket = &socket;
    uring_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
    sqe->user_data = reinterpret_cast<uint64_t>(uring_socket);
}

void IOUringCtrl::AddSocketWrite(BaseSocket &socket, uint16_t bid, std::size_t message_size, uint32_t flags)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    io_uring_prep_send(sqe, socket.GetSocketID(), &bufs_[bid], message_size, 0);
    io_uring_sqe_set_flags(sqe, flags);

    auto* uring_socket = socket.GetUringSocket();
    uring_socket->io_send.base_socket = &socket;
    uring_socket->io_send.io_type = SocketState::SOCK_STATE_SEND;
    uring_socket->io_send.bid = bid;
    sqe->user_data = reinterpret_cast<uint64_t>(uring_socket);
}

void IOUringCtrl::AddProvideBuf(BaseSocket &socket, uint16_t bid)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    io_uring_prep_provide_buffers(sqe, bufs_[bid], MAX_MESSAGE_LEN, 1, group_id_, bid);

    UringIOContext *io_context = GET_NET_OBJECT(UringIOContext);
    io_context->io_type = SocketState::SOCK_STATE_PROV_BUF;
    sqe->user_data = reinterpret_cast<uint64_t>(io_context);
}


};  // ToolBox

#endif  // __linux__






