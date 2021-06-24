#include "src/net_epoll/epoll_ctrl.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(NetEpoll)

CASE(TestNetEpoll){
    EpollCtrlMgr->CreateEpoll();
    EpollCtrlMgr->Destroy();


}

FIXTURE_END(TestNetEpoll)