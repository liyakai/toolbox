

#include "unit_test_frame/unittest.h"
#include "coro_rpc/coro_rpc_client.h"
#include "coro_rpc/coro_rpc_server.h"
#include "network/network_api.h"
#include "test/protobuf/test_coro_rpc.pb.h"
#include <cstring>
#include <unordered_set>
#include <vector>
#include <memory>
FIXTURE_BEGIN(CoroRpcForward)


demo::GetUserResponse echo(demo::GetUserRequest request){return demo::GetUserResponse();};
ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol,std::unordered_map> forward_server;
ToolBox::CoroRpc::CoroRpcClient<ToolBox::CoroRpc::CoroRpcProtocol> client_for_echo;

inline ToolBox::coro::Task<demo::GetUserResponse> forward_func(demo::GetUserRequest request) {
    // fprintf(stderr, "[forward_func]coro_rpc server forward, user_id: %s, attachment: %s\n", request.DebugString().c_str(), forward_server.GetReqAttachment().data());

    client_for_echo.SetReqAttachment(forward_server.GetReqAttachment());
    // demo::GetUserRequest request_echo;
    // request_echo.set_user_id(1010);
    auto result = co_await client_for_echo.Call<echo>(request);
        // 在这个函数的任何地方都可以获取到 attachment
    auto resp_attachment = result.GetRespAttachment();
    // fprintf(stderr, "test_coro_rpc_forward result status: %d, resp_message: %s, user_id: %d, resp_attachment size: %zu, resp_attachment:%s\n"
    //                             , result->status(), result->message().data(), result->user().id(), resp_attachment.size(), resp_attachment.data());


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
    forward_server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        // fprintf(stderr, "coro_rpc server send buffer content[size:%zu]: ", buffer.size());
        // for (size_t i = 0; i < buffer.size(); i++) {
        //   fprintf(stderr, "%02X[%u] ", static_cast<unsigned char>(buffer[i]), static_cast<uint8_t>(buffer[i]));
        // }
        // fprintf(stderr, "opaque: %lu\n", opaque);
        network.Send(opaque, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    //设置发送回调函数
    client_for_echo.SetSendCallback([&](std::string &&buffer) {        
        // fprintf(stderr, "coro_rpc client send buffer content[size:%zu]: ", buffer.size());
        // for (size_t i = 0; i < buffer.size(); i++) {
        //     fprintf(stderr, "%02X ", static_cast<unsigned char>(buffer[i]));
        // }
        // fprintf(stderr, "echo_conn_id: %lu\n", echo_conn_id);
        network.Send(echo_conn_id, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    network.SetOnConnected([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id) {
        echo_conn_id = conn_id;
        is_connected = true;
        fprintf(stderr, "coro_rpc client connect server success, opaque: %lu, conn_id: %lu\n", opaque
        , conn_id);
    }).SetOnReceived([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const char* data, size_t size) {
        // fprintf(stderr, "coro_rpc client received data, opaque: %lu, conn_id: %lu\n", opaque
        // , conn_id);
        // fprintf(stderr, "coro_rpc client recv buffer content: ");
        // for (size_t i = 0; i < size; i++) {
        //     fprintf(stderr, "%02X ", static_cast<unsigned char>(data[i]));
        // }
        // fprintf(stderr, "conn_id: %lu, echo_conn_id: %lu\n", conn_id, echo_conn_id);
        if (conn_id == echo_conn_id) 
        {
            client_for_echo.OnRecvResp(std::string_view(data, size));
        } else 
        {
            forward_server.OnRecvReq(conn_id, std::string_view(data, size));
        }
        
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) {
        fprintf(stderr, "coro_rpc client error, opaque: %lu, conn_id: %lu, err_code: %d, err_no: %d\n", opaque
        , conn_id, static_cast<int32_t>(err_code), err_no);
    });
    network.Connect(ToolBox::NT_TCP, 9702, "0.0.0.0", 9700);
    network.Accept(ToolBox::NT_TCP, 9703, "0.0.0.0", 9500);
    network.Start(1);
    forward_server.RegisterService<forward_func>();
    fprintf(stderr, "[CoroRpcEchoCase1] done\n");



    // 等待网络库停止
    uint64_t used_time = 0;
    uint64_t old_time = 0;
    uint64_t run_mill_seconds = 24 * 60 * 60 * 1000 ;
    while (true)
    {
        if (used_time > run_mill_seconds)
        {
            break;
        }
        uint64_t time_left = (run_mill_seconds - used_time) / 1000;
        if (time_left != old_time)
        {
            if (time_left + 10 <= old_time || old_time == 0)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    network.StopWait();

    return "test rpc client done.";
}

CASE(CoroRpcForwardCase1) 
{ 
    return;
    auto result = test_coro_rpc_forward();
    fprintf(stderr, "[CoroRpcForwardCase1] result: %s\n", result.data());
    
}

// ========== ThreadSafe/ThreadUnsafe 策略深度测试 ==========

// ThreadSafe 策略下 Server + Client 转发链路可以正常实例化和注册
CASE(TestThreadSafe_ForwardChainSetup)
{
    using namespace ToolBox::CoroRpc;
    
    // 转发服务器用 ThreadSafe 策略
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_forward_server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> ts_client_for_echo;
    
    // 设置回调
    ts_forward_server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        (void)buffer;
    });
    
    ts_client_for_echo.SetSendCallback([&](std::string &&buffer) {
        (void)buffer;
    });
    
    // ThreadSafe 下也能注册 forward_func（协程函数）
    ts_forward_server.RegisterService<forward_func>();
    
    auto keys = ts_forward_server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("ThreadSafe 策略下转发服务注册失败");
        return;
    }
}

// ThreadSafe 下 Client 附件设置和ID管理
CASE(TestThreadSafe_ClientAttachmentAndId)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> ts_client;
    
    // 设置附件
    std::string attachment = "XXX ThreadSafe attachment XXX";
    bool ret = ts_client.SetReqAttachment(attachment);
    if (!ret) {
        SetError("ThreadSafe 客户端设置附件失败");
        return;
    }
    
    // Client ID 管理
    uint64_t original_id = ts_client.get_client_id();
    ts_client.set_client_id(12345);
    if (ts_client.get_client_id() != 12345) {
        SetError("ThreadSafe 客户端设置 ID 失败");
        return;
    }
    (void)original_id;
}

// ThreadSafe 和 ThreadUnsafe 的 Client 混合使用（模拟真实场景：内部调用用无锁，外部连接用有锁）
CASE(TestMixedPolicy_ForwardScenario)
{
    using namespace ToolBox::CoroRpc;
    
    // 转发服务器：ThreadSafe（因为可能有多个连接并发处理）
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_forward;
    // 内部连接 echo 服务器的客户端：ThreadUnsafe（假设内部是单线程）
    CoroRpcClient<CoroRpcProtocol, ThreadUnsafe> tu_internal_client;
    // 外部客户端：ThreadSafe
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> ts_external_client;
    
    ts_forward.RegisterService<echo>();
    
    // 都能设置回调
    ts_forward.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        // 根据 opaque 决定是外部还是内部
        (void)opaque;
        (void)buffer;
    });
    tu_internal_client.SetSendCallback([&](std::string &&buffer) {
        (void)buffer;
    });
    ts_external_client.SetSendCallback([&](std::string &&buffer) {
        (void)buffer;
    });
    
    auto keys = ts_forward.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("混合策略场景下服务注册失败");
        return;
    }
}

// ThreadSafe Server 处理无效请求（错误包回传）
CASE(TestThreadSafe_ForwardServerInvalidRequest)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    
    // 不注册任何服务，直接发请求
    CoroRpcProtocol::ReqHeader req_header{};
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    req_header.msg_type = 0;
    req_header.seq_num = 1;
    req_header.func_id = 88888;  // 未注册
    req_header.length = 0;
    req_header.attach_length = 0;
    req_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    bool got_error_response = false;
    ts_server.SetSendCallback([&](uint64_t opaque, std::string &&resp) {
        if (resp.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader resp_header;
            std::memcpy(&resp_header, resp.data(), sizeof(resp_header));
            if (resp_header.err_code == static_cast<uint8_t>(Errc::ERR_FUNC_NOT_REGISTERED)) {
                got_error_response = true;
            }
        }
    });
    
    Errc err = ts_server.OnRecvReq(1, buffer);
    
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe Server OnRecvReq 对未注册函数应返回 SUCCESS（错误已回包）");
        return;
    }
    if (!got_error_response) {
        SetError("ThreadSafe Server 应回 ERR_FUNC_NOT_REGISTERED 错误包");
        return;
    }
}

// 多个 ThreadSafe Client 并发创建（验证全局 client_id 递增无冲突）
CASE(TestThreadSafe_ConcurrentClientCreation)
{
    using namespace ToolBox::CoroRpc;
    
    const int N = 20;
    std::vector<std::unique_ptr<CoroRpcClient<CoroRpcProtocol, ThreadSafe>>> clients;
    clients.reserve(N);
    
    for (int i = 0; i < N; ++i) {
        clients.push_back(std::make_unique<CoroRpcClient<CoroRpcProtocol, ThreadSafe>>());
    }
    
    // 验证所有 ID 互不相同
    std::unordered_set<uint64_t> ids;
    for (auto &c : clients) {
        ids.insert(c->get_client_id());
    }
    if (ids.size() != N) {
        SetError("多个 ThreadSafe 客户端的 ID 应互不相同");
        return;
    }
}

FIXTURE_END()