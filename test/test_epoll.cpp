#include "net_epoll/net_epoll.h"
#include "tools/unit_test_frame/unittest.h"

FIXTURE_BEGIN(NetEpoll)

CASE(TestNetEpoll){
    EpollCtrlMgr->CreateEpoll();
    EpollCtrlMgr->Destroy();


}

FIXTURE_END(TestNetEpoll)