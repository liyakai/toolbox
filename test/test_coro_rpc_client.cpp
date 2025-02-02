

#include "unit_test_frame/unittest.h"
#include "tools/coro_rpc/coro_rpc_client.h"
#include "network/network_api.h"
#include "test/protobuf/test_coro_rpc.pb.h"
FIXTURE_BEGIN(CoroRpcClient)

demo::GetUserResponse echo(demo::GetUserRequest request);

ToolBox::coro::Task<std::string_view> test_coro_rpc_client(ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> &client) {
    client.SetReqAttachment("This is a attachment");
    demo::GetUserRequest request;
    request.set_user_id(1010);
    auto result = co_await client.Call<echo>(request);
        // 在这个函数的任何地方都可以获取到 attachment
    auto resp_attachment = result.get_attachment();
    fprintf(stderr, "test_coro_rpc_client result status: %d, resp_message: %s, user_id: %d, resp_attachment size: %zu, resp_attachment:%s\n"
                                , result->status(), result->message().data(), result->user().id(), resp_attachment.size(), resp_attachment.data());
    co_return "test rpc client done.";
}

std::string_view test_coro_rpc_client() {
        //声明RPC 客户端
    ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> client;
    //声明网络库句柄
    ToolBox::Network network;
    uint64_t server_conn_id = 0;
    bool is_connected = false;
    //设置发送缓冲区回调函数
    client.SetSendCallback([&](std::string &&buffer) {        
        fprintf(stderr, "coro_rpc client send buffer content[size:%zu]: ", buffer.size());
        for (size_t i = 0; i < buffer.size(); i++) {
            fprintf(stderr, "%02X ", static_cast<unsigned char>(buffer[i]));
        }
        fprintf(stderr, "\n");
        network.Send(server_conn_id, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    network.SetOnConnected([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id) {
        server_conn_id = conn_id;
        is_connected = true;
        fprintf(stderr, "coro_rpc client connect server success, opaque: %lu, conn_id: %lu\n", opaque
        , conn_id);
    }).SetOnReceived([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const char* data, size_t size) {
        fprintf(stderr, "coro_rpc client received data, opaque: %lu, conn_id: %lu\n", opaque
        , conn_id);
        fprintf(stderr, "coro_rpc client recv buffer content: ");
        for (size_t i = 0; i < size; i++) {
            fprintf(stderr, "%02X ", static_cast<unsigned char>(data[i]));
        }
        fprintf(stderr, "\n");
        client.OnRecvResp(std::string_view(data, size));
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) {
        fprintf(stderr, "coro_rpc client error, opaque: %lu, conn_id: %lu, err_code: %d, err_no: %d\n", opaque
        , conn_id, err_code, err_no);
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

                // 发送数据
                if (is_connected) {
                    auto coro_result = test_coro_rpc_client(client);
                    coro_result.then([](std::string_view result) {
                        fprintf(stderr, "[CoroRpcClientCase1] ####################result: %s\n", result.data());
                    });
                }
            }
        }
        ToolBox::TimerMgr->Update();
        network.Update();
        fprintf(stderr, "coro_rpc client update\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    network.StopWait();

    return "test rpc client done.";
}

CASE(CoroRpcClientCase1) 
{ 
    return;
    auto result = test_coro_rpc_client();
    fprintf(stderr, "[CoroRpcClientCase1] result: %s\n", result.data());
    
}

FIXTURE_END()