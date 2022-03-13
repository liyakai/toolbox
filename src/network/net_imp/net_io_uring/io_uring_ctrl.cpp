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
        printf("IORING_FEAT_FAST_POLL not available in the kernel, quiting...\n");
        exit(0);
    }

    // check if buffer selection is supported
    struct io_uring_probe *probe;
    probe = io_uring_get_probe_ring(&ring_);
    if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS)) {
        printf("Buffer select not supported, skipping...\n");
        exit(0);
    }
    free(probe);

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

// int IOUringCtrl::EpollWait(int msec)
// {
//     return epoll_wait(epoll_fd_, events_, max_events_, msec);
// }

};  // ToolBox

#endif  // __linux__






