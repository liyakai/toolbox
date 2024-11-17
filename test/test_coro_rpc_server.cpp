

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_server.h"
#include "tools/timer.h"
FIXTURE_BEGIN(CoroRpcServer)

inline std::string_view echo(std::string_view str)
{
    return str;
}

ToolBox::coro::Task<std::string_view> test_coro_rpc_server() {
    std::thread t([]() {
        while (true) {
            ToolBox::TimerMgr->Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    t.detach();
    ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol, std::unordered_map> server;
    server.template RegisterService<echo>();
    // auto result = co_await server.call<echo>("hello CoroRpc, world");
    // fprintf(stderr, "test_coro_rpc_client result: %s\n", result.data());

    co_return "hello CoroRpc, world";
}

CASE(CoroRpcServerCase1) 
{ 
    auto task = test_coro_rpc_server();
    auto result = task.get_result();
    fprintf(stderr, "[CoroRpcServerCase1] result: %s\n", result.data());

    std::this_thread::sleep_for(std::chrono::milliseconds(100000));
}

FIXTURE_END()