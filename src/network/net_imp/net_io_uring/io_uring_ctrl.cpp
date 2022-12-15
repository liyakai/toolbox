#include "io_uring_ctrl.h"
#ifdef LINUX_IO_URING

#include "network_def.h"
#include <unistd.h>


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
        time_t time_stamp = time(0);    // 时间戳
        int32_t count = URingWait(URING_WAIT_MSECONDS);
        if (count < 0)
        {
            NetworkLogError("[Network] the number of SQEs submitted:%u", count);
            return false;
        }
        NetworkLogTrace("[Network] the number of SQEs submitted:%u", count);
        // io_uring 设置了两个ringbuffer：
        // sq(submission queue):存放提交的IO请求,应用层为生产者操作tail,内核为消费者操作head.其中的entry称为sqe.
        // cq(completion queue):存放处理完成的IO请求,内核为生产者操作tail,应用层为消费者操作head.其中的entry称为cqe.
        struct io_uring_cqe* cqe;
        uint16_t head = 0;
        uint16_t cqe_count = 0;

        // go through all CQEs
        io_uring_for_each_cqe(ring_, head, cqe)
        {
            ++cqe_count;
            // usleep(10);
            UringIOContext* uring_io = reinterpret_cast<UringIOContext*>(cqe->user_data);
            if (nullptr == uring_io)
            {
                NetworkLogError("[Network] convert user_data to UringIOContext failed. cqe:%p, user_data:%p", cqe, cqe->user_data);
                break;
            }
            BaseSocket* socket = uring_io->base_socket;
            if (nullptr == socket)
            {
                NetworkLogError("[Network] base_socket is nullpter. cqe:%p, user_data:%p", cqe, cqe->user_data);
                break;
            }
            if (cqe->res == -ENOBUFS)
            {
                NetworkLogError("[Network] bufs in automatic buffer selection empty, this should not happen....");
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                break;
            }
            // only read when there is no error, >= 0
            if (cqe->res < 0)
            {
                NetworkLogWarn("[Network] io_uring_for_each_cqe error: cqe->res:%d, socket id:%d,uring_io io_type:%d ", cqe->res, socket->GetSocketID(), uring_io->io_type);
                errno = -cqe->res;
                socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                break;
            }
            else if (cqe->res == 0)
            {
                NetworkLogDebug("[Network] io_uring_for_each_cqe error: cqe->res:%d, socket id:%d,uring_io io_type:%d ", cqe->res, socket->GetSocketID(), uring_io->io_type);
                if (SOCK_STATE_SEND == uring_io->io_type)
                {
                    NetworkLogDebug("[Network] io_uring_for_each_cqe connect success.");
                    socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
                }
                else if (SOCK_STATE_RECV == uring_io->io_type)
                {
                    NetworkLogDebug("[Network] io_uring_for_each_cqe socket disconnected.");
                    socket->UpdateEvent(SOCKET_EVENT_ERR, time_stamp);
                }

                break;
            }

            NetworkLogTrace("[Network] io_uring_for_each_cqe socket type:%d, uring_io type:%d, io_uring_for_each_cqecqe_count:%u ", socket->GetSocketState(), uring_io->io_type, cqe_count);
            if (SOCK_STATE_PROV_BUF & uring_io->io_type)
            {
                NetworkLogInfo("[Network] SOCK_STATE_PROV_BUF recv message.");
            }
            else if (SOCK_STATE_LISTENING & uring_io->io_type)
            {
                NetworkLogDebug("[Network] SOCK_STATE_LISTENING on recvd connect. socket id:%d, socket state:%d", socket->GetSocketID(), socket->GetSocketState());
                int32_t sock_conn_fd = cqe->res;
                if (sock_conn_fd >= 0)
                {
                    // AddSocketRead(*socket, IOSQE_BUFFER_SELECT);
                    auto* uring_socket = socket->GetUringSocket();
                    uring_socket->accept_ex->socket_fd = sock_conn_fd;
                    socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
                }
            }
            else if (SOCK_STATE_RECV & uring_io->io_type)
            {
                NetworkLogTrace("[Network] SOCK_STATE_RECV on recvd data. len:%d", cqe->res);
                // int32_t bytes_read = cqe->res;
                // int32_t bid = cqe->flags >> 16;
                // bytes have been read into bufs, now add write to socket sqe
                // AddSocketWrite(*socket, bid, bytes_read, 0);
                auto* uring_socket = socket->GetUringSocket();
                uring_socket->io_recv.len = cqe->res;
                socket->UpdateEvent(SOCKET_EVENT_RECV, time_stamp);
            }
            else if (SOCK_STATE_SEND & uring_io->io_type)
            {
                NetworkLogTrace("[Network] SOCK_STATE_SEND has sended data.");
                // write has been completed, first re-add the buffer
                // AddProvideBuf(*socket, uring_io->bid);
                // add a new read for the existing connection
                // AddSocketWrite(*socket, uring_io->bid, MAX_MESSAGE_LEN, 0);
                int32_t bytes_write = cqe->res;
                auto* uring_socket = socket->GetUringSocket();
                uring_socket->io_send.len = bytes_write;
                socket->UpdateEvent(SOCKET_EVENT_SEND, time_stamp);
            }

        }
        io_uring_cq_advance(ring_, cqe_count);
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
            return AddSocketAccept(&socket, flags);
        }
        else if (SocketState::SOCK_STATE_ESTABLISHED == socket.GetSocketState())
        {
            return AddSocketRead(socket, 0);
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
        return AddSocketWrite(socket, 0, 0);
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

        NetworkLogDebug("[Network] AddSocketAccept add accept. socket id:%d, socket state:%d", socket->GetSocketID(), socket->GetSocketState());
        return true;
    }

    bool IOUringCtrl::AddSocketRead(BaseSocket& socket, uint32_t flags)
    {
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        if (!sqe)
        {
            NetworkLogError("[Network] get sqe from io_uring failed.");
            return false;
        }

        auto* uring_socket = socket.GetUringSocket();
        if (!uring_socket)
        {
            NetworkLogError("[Network] the socket get uring socket failed.");
            return false;
        }

        sqe->buf_group = group_id_;
        uring_socket->io_recv.base_socket = &socket;
        uring_socket->io_recv.io_type = SocketState::SOCK_STATE_RECV;
        sqe->user_data = reinterpret_cast<uint64_t>(&uring_socket->io_recv);

        io_uring_prep_recv(sqe, socket.GetSocketID(), uring_socket->io_recv.buf, uring_socket->io_recv.len, flags);
        // io_uring_sqe_set_flags(sqe, flags);

        return true;
    }

    bool IOUringCtrl::AddSocketWrite(BaseSocket& socket, uint16_t bid, uint32_t flags)
    {
        auto* uring_socket = socket.GetUringSocket();
        if (!uring_socket)
        {
            NetworkLogError("[Network] the socket get uring socket failed.");
            return false;
        }
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);

        uring_socket->io_send.base_socket = &socket;
        uring_socket->io_send.io_type = SocketState::SOCK_STATE_SEND;
        uring_socket->io_send.bid = bid;
        sqe->user_data = reinterpret_cast<uint64_t>( &uring_socket->io_send);

        io_uring_prep_send(sqe, socket.GetSocketID(), uring_socket->io_send.buf, uring_socket->io_send.len, flags);
        return true;
    }
};  // ToolBox

#endif  // LINUX_IO_URING






