

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_client.h"
FIXTURE_BEGIN(CoroRpcClient)

inline std::string_view echo(std::string_view str)
{
    return str;
}

ToolBox::coro::Task<std::string_view> test_coro_rpc_client() {
    std::thread t([]() {
        while (true) {
            ToolBox::TimerMgr->Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    t.detach();
    ToolBox::CoroRpc::CoroRpcClient client;
    auto result = co_await client.call<echo>("hello CoroRpc, world");
    fprintf(stderr, "test_coro_rpc_client result: %s\n", result.data());

    co_return result;
}

CASE(CoroRpcClientCase1) 
{ 
    auto task = test_coro_rpc_client();
    auto result = task.get_result();
    fprintf(stderr, "[CoroRpcClientCase1] result: %s\n", result.data());

    // std::this_thread::sleep_for(std::chrono::milliseconds(100000));
}

FIXTURE_END()