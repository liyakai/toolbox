

#include "unit_test_frame/unittest.h"
#include "coro_rpc/coro_rpc_client.h"
#include "coro_rpc/coro_rpc_server.h"
#include "network/network_api.h"
#include "test/protobuf/test_coro_rpc.pb.h"
FIXTURE_BEGIN(CoroRpcForward)


demo::GetUserResponse echo(demo::GetUserRequest request){return demo::GetUserResponse();};
ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol,std::unordered_map> forward_server;
ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> client_for_echo;

inline ToolBox::coro::Task<demo::GetUserResponse> forward_func(demo::GetUserRequest request) {
    fprintf(stderr, "[forward_func]coro_rpc server forward, user_id: %s, attachment: %s\n", request.DebugString().c_str(), forward_server.GetReqAttachment().data());

    client_for_echo.SetReqAttachment(forward_server.GetReqAttachment());
    // demo::GetUserRequest request_echo;
    // request_echo.set_user_id(1010);
    auto result = co_await client_for_echo.Call<echo>(request);
        // 在这个函数的任何地方都可以获取到 attachment
    auto resp_attachment = result.GetRespAttachment();
    fprintf(stderr, "test_coro_rpc_forward result status: %d, resp_message: %s, user_id: %d, resp_attachment size: %zu, resp_attachment:%s\n"
                                , result->status(), result->message().data(), result->user().id(), resp_attachment.size(), resp_attachment.data());


    forward_server.SetRespAttachmentFunc<forward_func>(
        [&]() -> std::string_view { return resp_attachment; });
    demo::GetUserResponse response;
    response.set_status(200);
    response.set_message("success");
    response.mutable_user()->set_id(request.user_id());
    //   fprintf(stderr, "[echo]coro_rpc server echo, response:%s\n", response.DebugString().c_str());
    co_return response;
}

ToolBox::coro::Task<std::string_view> test_coro_rpc_forward(ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> &client) {
    client.SetReqAttachment("XXX This is a attachment. XXX");
    demo::GetUserRequest request;
    request.set_user_id(1010);
    auto result = co_await client.Call<echo>(request);
        // 在这个函数的任何地方都可以获取到 attachment
    auto resp_attachment = result.GetRespAttachment();
    fprintf(stderr, "test_coro_rpc_forward result status: %d, resp_message: %s, user_id: %d, resp_attachment size: %zu, resp_attachment:%s\n"
                                , result->status(), result->message().data(), result->user().id(), resp_attachment.size(), resp_attachment.data());
    co_return "test rpc client done.";
}

std::string_view test_coro_rpc_forward() {


    //声明网络库句柄
    ToolBox::Network network;
    uint64_t echo_conn_id = 0;
    bool is_connected = false;
    forward_server.SetSendCallback([&](uint64_t opaque, std::string_view &&buffer) {
        fprintf(stderr, "coro_rpc server send buffer content[size:%zu]: ", buffer.size());
        for (size_t i = 0; i < buffer.size(); i++) {
          fprintf(stderr, "%02X[%u] ", static_cast<unsigned char>(buffer[i]), static_cast<uint8_t>(buffer[i]));
        }
        fprintf(stderr, "opaque: %lu\n", opaque);
        network.Send(opaque, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    //设置发送回调函数
    client_for_echo.SetSendCallback([&](std::string &&buffer) {        
        fprintf(stderr, "coro_rpc client send buffer content[size:%zu]: ", buffer.size());
        for (size_t i = 0; i < buffer.size(); i++) {
            fprintf(stderr, "%02X ", static_cast<unsigned char>(buffer[i]));
        }
        fprintf(stderr, "echo_conn_id: %lu\n", echo_conn_id);
        network.Send(echo_conn_id, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    network.SetOnConnected([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id) {
        echo_conn_id = conn_id;
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
        fprintf(stderr, "conn_id: %lu, echo_conn_id: %lu\n", conn_id, echo_conn_id);
        if (conn_id == echo_conn_id) 
        {
            client_for_echo.OnRecvResp(std::string_view(data, size));
        } else 
        {
            forward_server.OnRecvReq(conn_id, std::string_view(data, size));
        }
        
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) {
        fprintf(stderr, "coro_rpc client error, opaque: %lu, conn_id: %lu, err_code: %d, err_no: %d\n", opaque
        , conn_id, err_code, err_no);
    });
    network.Connect(ToolBox::NT_TCP, 9702, "0.0.0.0", 9700);
    network.Accept(ToolBox::NT_TCP, 9703, "0.0.0.0", 9500);
    network.Start(1);
    forward_server.RegisterService<forward_func>();
    fprintf(stderr, "[CoroRpcEchoCase1] done\n");



    // 等待网络库停止
    uint64_t used_time = 0;
    uint64_t old_time = 0;
    uint64_t run_mill_seconds = 1 * 60 * 60 * 1000 * 1000;
    while (true)
    {
        if (used_time > run_mill_seconds)
        {
            break;
        }
        uint64_t time_left = (run_mill_seconds - used_time) / 1000000;
        if (time_left != old_time)
        {
            if (time_left + 10*1000 <= old_time || old_time == 0)
            {
                fprintf(stderr, "距离网络库停止还有%lu秒 \n", time_left);
                old_time = time_left;

                // 发送数据
                if (is_connected) {
                    // auto coro_result = test_coro_rpc_forward(client_for_echo);
                    // coro_result.then([](std::string_view result) {
                    //     fprintf(stderr, "[CoroRpcForwardCase1] ####################result: %s\n", result.data());
                    // });
                }
            }
        }
        ToolBox::TimerMgr->Update();
        network.Update();
        // fprintf(stderr, "coro_rpc client update\n");
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        used_time += 100*1000;
    }

    network.StopWait();

    return "test rpc client done.";
}

CASE(CoroRpcForwardCase1) 
{ 
    // return;
    auto result = test_coro_rpc_forward();
    fprintf(stderr, "[CoroRpcForwardCase1] result: %s\n", result.data());
    
}

FIXTURE_END()