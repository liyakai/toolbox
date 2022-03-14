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
    io_uring_prep_provide_buffers(sqe, bufs, MAX_MESSAGE_LEN, BUFFERS_COUNT, group_id_, 0);

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

int32_t IOUringCtrl::EpollWait(int msec)
{
    return io_uring_submit_and_wait(&ring_, msec);
}

};  // ToolBox

#endif  // __linux__






