

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_client.h"
FIXTURE_BEGIN(CoroRpcClient)

inline std::string_view echo(std::string_view str)
{
    return str;
}

ToolBox::coro::Task<std::string_view> test_coro_rpc_client() {
    ToolBox::coro_rpc::CoroRpcClient client;
    auto result = co_await client.call<echo>("hello coro_rpc, world");
    std::cout << "result: " << result << std::endl;
    co_return result;
}

CASE(CoroRpcClientCase1) 
{ 
    // test_coro_rpc_client();
}

FIXTURE_END()