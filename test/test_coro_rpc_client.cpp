

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_client.h"
#include "network/network_api.h"

FIXTURE_BEGIN(CoroRpcClient)

inline std::string_view echo(std::string_view str)
{
    return str;
}

ToolBox::coro::Task<std::string_view> test_coro_rpc_client(ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> &client) {
    client.set_req_attachment("This is a attachment");
    auto result = co_await client.call<echo>("hello CoroRpc, world");
    std::string_view resp_attachment = client.get_resp_attachment();
    fprintf(stderr, "test_coro_rpc_client result: %s,resp_attachment:%s\n", result.data(), resp_attachment.data());

    co_return result;
}

CASE(CoroRpcClientCase1) 
{ 
    //声明RPC 客户端
    ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> client;
    //声明网络库句柄
    ToolBox::Network network;
    uint64_t server_conn_id = 0;
    //设置发送缓冲区回调函数
    client.set_send_callback([&](std::vector<std::byte> &&buffer) {        
        fprintf(stderr, "coro_rpc client send buffer content: ");
        for (size_t i = 0; i < buffer.size(); i++) {
            fprintf(stderr, "%02X ", static_cast<unsigned char>(buffer[i]));
        }
        fprintf(stderr, "\n");
        network.Send(server_conn_id, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });

    network.SetOnConnected([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id) {
        server_conn_id = conn_id;
        fprintf(stderr, "coro_rpc client connect server success, opaque: %lu, conn_id: %lu\n", opaque
        , conn_id);
        auto task = test_coro_rpc_client(client);
        auto result = task.get_result();
        fprintf(stderr, "[CoroRpcClientCase1] result: %s\n", result.data());
    });
    network.Connect(ToolBox::NT_TCP, 9702, "0.0.0.0", 9700);
    network.Start(1);


    // 等待网络库停止
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 1 * 60 * 60 * 1000;
    while (true)
    {
        if (used_time > run_mill_seconds)
        {
            break;
        }
        uint32_t time_left = (run_mill_seconds - used_time) / 1000;
        if (time_left != old_time)
        {
            if (time_left + 10 <= old_time || old_time == 0)
            {
                fprintf(stderr, "距离网络库停止还有%d秒 \n", time_left);
                old_time = time_left;
            }
        }
        ToolBox::TimerMgr->Update();
        network.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    network.StopWait();
}

FIXTURE_END()