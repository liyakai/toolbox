#include "io_uring_ctrl.h"
#ifdef LINUX_IO_URING

namespace ToolBox
{

    IOUringCtrl::IOUringCtrl(uint max_events)
        : max_events_(max_events)
    {
        ring_ = new io_uring();
    }

    IOUringCtrl::~IOUringCtrl()
    {
        io_uring_queue_exit(ring_);
    }

    bool IOUringCtrl::CreateIOMultiplexing()
    {
        NetworkLogInfo("[Network] start CreateIOMultiplexing.");

        // initialize io_uring
        struct io_uring_params params;
        // struct io_uring ring;
        memset(&params, 0, sizeof(params));
        int32_t ret = io_uring_queue_init_params(max_events_, ring_, &params);
        if (ret < 0)
        {
            NetworkLogError("[Network] io_uring_queue_init_params failed. ret:%d", ret);
            return false;
        }

        // check if IORING_FEAT_FAST_POLL is supported
        if (!(params.features & IORING_FEAT_FAST_POLL))
        {
            NetworkLogError("[Network] IORING_FEAT_FAST_POLL not available in the kernel.");
            return false;
        }

        // check if buffer selection is supported
        struct io_uring_probe* probe;
        probe = io_uring_get_probe_ring(ring_);
        if (!probe)
        {
            NetworkLogError("[Network] Buffer select not supported, skipping...");
            return false;
        }
        if (!io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS))
        {
            NetworkLogError("[Network] Buffer select not supported, skipping...");
            return false;
        }
        free(probe);

        // register buffers for buffer selection
        struct io_uring_sqe* sqe;
        struct io_uring_cqe* cqe;

        sqe = io_uring_get_sqe(ring_);
        io_uring_prep_provide_buffers(sqe, bufs_, MAX_MESSAGE_LEN, BUFFERS_COUNT, group_id_, 0);

        io_uring_submit(ring_);
        io_uring_wait_cqe(ring_, &cqe);
        if (cqe->res < 0)
        {
            NetworkLogError("[Network] cqe->res = %d\n", cqe->res);
            return false;
        }
        io_uring_cqe_seen(ring_, cqe);

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

    bool IOUringCtrl::OperEvent(BaseSocket& socket, EventOperType op_type, int32_t event_type)
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
        return io_uring_submit_and_wait(ring_, 1);
    }

    bool IOUringCtrl::RunOnce()
    {
        NetworkLogInfo("[Network] RunOnce");
        time_t time_stamp = time(0);    // 时间戳
        int32_t count = URingWait(URING_WAIT_MSECONDS);
        if (count < 0)
        {
            NetworkLogError("[Network] the number of SQEs submitted:%u", count);
            return false;
        }
        NetworkLogInfo("[Network] the number of SQEs submitted:%u", count);
        //sleep(2);
        // io_uring 设置了两个ringbuffer：
        // sq(submission queue):存放提交的IO请求,应用层为生产者操作tail,内核为消费者操作head.其中的entry称为sqe.
        // cq(completion queue):存放处理完成的IO请求,内核为生产者操作tail,应用层为消费者操作head.其中的entry称为cqe.
        struct io_uring_cqe* cqe;
        uint16_t head = 0;
        uint16_t cqe_count = 0;

        // go through all CQEs
        io_uring_for_each_cqe(ring_, head, cqe)
        {
            NetworkLogInfo("[Network] io_uring_for_each_cqecqe_count:%u", cqe_count);
            ++cqe_count;
            UringIOContext* uring_io = reinterpret_cast<UringIOContext*>(cqe->user_data);
            if (nullptr == uring_io)
            {
                NetworkLogError("[Network] convert user_data to UringIOContext failed. user_data:%p", cqe->user_data);
                continue;
            }
            BaseSocket* socket = uring_io->base_socket;
            if (nullptr == socket)
            {
                NetworkLogError("[Network] base_socket is nullpter.");
                continue;
            }
            if (cqe->res == -ENOBUFS)
            {
                // fprintf(stdout, "bufs in automatic buffer selection empty, this should not happen...\n");
                // fflush(stdout);
                NetworkLogError("[Network] bufs in automatic buffer selection empty, this should not happen....");
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                continue;
            }
            NetworkLogInfo("[Network] io_uring_for_each_cqe socket type:%d, uring_io type:%d ", socket->GetSocketState(), uring_io->io_type);
            NetworkLogInfo("[Network] io_uring_for_each_cqe socket type:, uring_io type:%d ", uring_io->io_type);
            if (SOCK_STATE_PROV_BUF & uring_io->io_type)
            {
                NetworkLogInfo("[Network] SOCK_STATE_PROV_BUF recv message.");
                if (cqe->res < 0)
                {
                    socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                }
            }
            else if (SOCK_STATE_LISTENING & uring_io->io_type)
            {
                NetworkLogInfo("[Network] SOCK_STATE_LISTENING on recvd connect.");
                int32_t sock_conn_fd = cqe->res;
                // only read when there is no error, >= 0
                if (sock_conn_fd >= 0)
                {
                    // AddSocketRead(*socket, IOSQE_BUFFER_SELECT);
                    auto* uring_socket = socket->GetUringSocket();
                    uring_socket->accept_ex->socket_fd = sock_conn_fd;
                    socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
                }

                // new connected client; read data from socket and re-add accept to monitor for new connections
                // AddSocketAccept(socket, 0);
            }
            else if (SOCK_STATE_RECV & uring_io->io_type)
            {
                NetworkLogInfo("[Network] SOCK_STATE_RECV on recvd data.");
                int32_t bytes_read = cqe->res;
                int32_t bid = cqe->flags >> 16;
                if (cqe->res <= 0)
                {
                    // read failed, re-add the buffer
                    // AddProvideBuf(*socket, bid);
                    // connection closed or error
                    socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                }
                else
                {
                    // bytes have been read into bufs, now add write to socket sqe
                    // AddSocketWrite(*socket, bid, bytes_read, 0);
                    auto* uring_socket = socket->GetUringSocket();
                    uring_socket->io_recv.len = cqe->res;
                    socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
                }
            }
            else if (SOCK_STATE_SEND & uring_io->io_type)
            {
                NetworkLogInfo("[Network] SOCK_STATE_SEND has sended data.");
                // write has been completed, first re-add the buffer
                // AddProvideBuf(*socket, uring_io->bid);
                // add a new read for the existing connection
                // AddSocketWrite(*socket, uring_io->bid, MAX_MESSAGE_LEN, 0);
            }

        }
        NetworkLogDebug("[Network] io_uring_cq_advance begin.");
        io_uring_cq_advance(ring_, cqe_count);
        NetworkLogDebug("[Network] io_uring_cq_advance end.");
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

    bool IOUringCtrl::OnRecv(BaseSocket& socket, uint32_t flags)
    {
        socket.ResetRecvAsyncSocket();
        auto* uring_socket = socket.GetUringSocket();
        if (SocketState::SOCK_STATE_LISTENING == socket.GetSocketState())
        {
            auto& accept_ex = uring_socket->accept_ex;
            if (nullptr == accept_ex)
            {
                accept_ex = new AcceptEx_t;
            }
            AddSocketAccept(&socket, flags);
        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            NetworkLogInfo("[Network] on recvd data.");
            AddSocketRead(socket, 0);
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

    bool IOUringCtrl::OnSend(BaseSocket& socket, uint32_t flags)
    {
        socket.ResetSendAsyncSocket();
        auto& io_send = socket.GetUringSocket()->io_send;
        if (SocketState::SOCK_STATE_CONNECTING == socket.GetSocketState())
        {
            io_send.io_type = SocketState::SOCK_STATE_CONNECTING;
        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            io_send.io_type = SocketState::SOCK_STATE_SEND;
        }
        // struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
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

    bool IOUringCtrl::AddSocketAccept(BaseSocket* socket, uint32_t flags)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        if (!sqe)
        {
            NetworkLogError("[Network] get sqe from io_uring failed.");
            return false;
        }

        auto* uring_socket = socket->GetUringSocket();
        if (!uring_socket)
        {
            NetworkLogError("[Network] the socket get uring socket failed.");
            return false;
        }
        uring_socket->io_recv.base_socket = socket;
        uring_socket->io_recv.io_type = SocketState::SOCK_STATE_LISTENING;
        sqe->user_data = reinterpret_cast<uint64_t>(&uring_socket->io_recv);


        io_uring_prep_accept(sqe, socket->GetSocketID(), reinterpret_cast<struct sockaddr*>(&client_addr), &client_len, flags);
        io_uring_sqe_set_flags(sqe, flags);
        // io_uring_prep_poll_add(sqe, socket.GetSocketID(), POLLIN);

        NetworkLogInfo("[Network] AddSocketAccept add accept.");
        return true;
    }

    void IOUringCtrl::AddSocketRead(BaseSocket& socket, uint32_t flags)
    {
        auto* uring_socket = socket.GetUringSocket();
        if (!uring_socket)
        {
            NetworkLogError("[Network] the socket get uring socket failed.");
            return;
        }
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        sqe->buf_group = group_id_;
        uring_socket->io_recv.base_socket = &socket;
        uring_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
        sqe->user_data = reinterpret_cast<uint64_t>(&uring_socket->io_recv);

        io_uring_prep_recv(sqe, socket.GetSocketID(), uring_socket->io_recv.buf, uring_socket->io_recv.len, flags);
        io_uring_sqe_set_flags(sqe, flags);
    }

    void IOUringCtrl::AddSocketWrite(BaseSocket& socket, uint16_t bid, std::size_t message_size, uint32_t flags)
    {
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        io_uring_prep_send(sqe, socket.GetSocketID(), &bufs_[bid], message_size, 0);
        io_uring_sqe_set_flags(sqe, flags);

        auto* uring_socket = socket.GetUringSocket();
        // uring_socket->io_send.base_socket = &socket;
        uring_socket->io_send.io_type = SocketState::SOCK_STATE_SEND;
        uring_socket->io_send.bid = bid;
        sqe->user_data = reinterpret_cast<uint64_t>(&socket);
    }

    void IOUringCtrl::AddProvideBuf(BaseSocket& socket, uint16_t bid)
    {
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        io_uring_prep_provide_buffers(sqe, bufs_[bid], MAX_MESSAGE_LEN, 1, group_id_, bid);

        UringIOContext* io_context = GET_NET_OBJECT(UringIOContext);
        io_context->io_type = SocketState::SOCK_STATE_PROV_BUF;
        sqe->user_data = reinterpret_cast<uint64_t>(io_context);
    }
};  // ToolBox

#endif  // LINUX_IO_URING






