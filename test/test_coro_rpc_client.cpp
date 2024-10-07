

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_client.h"
FIXTURE_BEGIN(CoroRpcClient)

CASE(CoroRpcClientCase1) { ToolBox::coro_rpc::CoroRpcClient client; }

FIXTURE_END()