#include "src/net_epoll/epoll_ctrl.h"
#include "unit_test_frame/unittest.h"
#ifdef __linux__
FIXTURE_BEGIN(NetEpoll)

CASE(TestNetEpoll){


    EpollCtrlMgr->CreateEpoll();
    EpollCtrlMgr->Destroy();


}

FIXTURE_END(TestNetEpoll)

#endif // __linux__