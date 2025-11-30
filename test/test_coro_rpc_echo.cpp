

#include "network/network_api.h"
#include "test/protobuf/test_coro_rpc.pb.h"
#include "coro_rpc/coro_rpc_server.h"
#include "tools/timer.h"
#include "unit_test_frame/unittest.h"
#include <thread>
FIXTURE_BEGIN(CoroRpcEcho)

// 声明 rpc server
ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol,std::unordered_map> server;

inline demo::GetUserResponse echo(demo::GetUserRequest request) {
    //   fprintf(stderr, "[echo]coro_rpc server echo, user_id: %d\n", request.user_id());
    server.SetRespAttachmentFunc<echo>(
        []() -> std::string_view { return server.GetReqAttachment(); });
    demo::GetUserResponse response;
    response.set_status(200);
    response.set_message("success");
    response.mutable_user()->set_id(request.user_id());
    auto attachment = server.GetReqAttachment();
    fprintf(stderr, "[echo]coro_rpc server echo, response:%s, attachment:%s.\n", response.DebugString().c_str(), attachment.data());
    return response;
}

class Service {
public:
  demo::GetUserResponse class_echo(demo::GetUserRequest request) {
    fprintf(stderr, "[echo]coro_rpc server echo, user_id: %d\n", request.user_id());
    server.SetRespAttachmentFunc<echo>(
        []() -> std::string_view { return server.GetReqAttachment(); });
    demo::GetUserResponse response;
    response.set_status(200);
    response.set_message("success");
    response.mutable_user()->set_id(request.user_id());
    fprintf(stderr, "[echo]coro_rpc server echo, response:%s\n", response.DebugString().c_str());
    return response;
  }

ToolBox::coro::Task<std::string_view> test_coro_rpc_server(ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol, std::unordered_map> &server) 
{
    server.template RegisterService<echo>();
    co_return "hello CoroRpc, world";
}

};

CASE(CoroRpcEchoCase1) 
{ 
    return;

    //声明网络库句柄
    ToolBox::Network network;
        //设置发送缓冲区回调函数
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        // fprintf(stderr, "coro_rpc server send buffer content[size:%zu]: ", buffer.size());
        // for (size_t i = 0; i < buffer.size(); i++) {
        //   fprintf(stderr, "%02X[%u] ", static_cast<unsigned char>(buffer[i]), static_cast<uint8_t>(buffer[i]));
        // }
        // fprintf(stderr, "opaque: %lu\n", opaque);
        network.Send(opaque, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    });
    network.SetOnReceived([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const char* data, size_t size) {
        // fprintf(stderr, "coro_rpc server received data, opaque: %lu, conn_id: %lu\n", opaque, conn_id);
        // fprintf(stderr, "coro_rpc server recv buffer content[size:%zu]: ", size);
        // for (size_t i = 0; i < size; i++) {
        //   fprintf(stderr, "%02X ", static_cast<unsigned char>(data[i]));
        // }
        // fprintf(stderr, "opaque: %lu\n", opaque);
        server.OnRecvReq(conn_id, std::string_view(data, size));

    }).SetOnAccepted([&](ToolBox::NetworkType type, uint64_t opaque, int32_t fd) {
        fprintf(stderr, "coro_rpc server accepted, opaque: %lu, fd: %d\n", opaque, fd);
    }).SetOnBinded([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const std::string & ip, uint16_t port)
    {
        printf("[TestRpcEcho] Server has established listening port, network type:%d, connection tag:%lu, connection ID:%lu, ip:%s, port:%d\n", type, opaque, conn_id, ip.c_str(), port);
    }).SetOnAccepting([](ToolBox::NetworkType type, uint64_t opaque, int32_t fd)
    {
        printf("[TestRpcEcho] Preparing to add new connection to IO multiplexing, network type:%d, connection tag:%lu, fd:%d\n", type, opaque, fd);
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) {
        fprintf(stderr, "coro_rpc server error, opaque: %lu, conn_id: %lu, err_code: %d, err_no: %d\n", opaque
        , conn_id, static_cast<int32_t>(err_code), err_no);
      });
    network.Accept(ToolBox::NT_TCP, 9701, "0.0.0.0", 9700);
    network.Start(2);

    server.RegisterService<echo>();
    fprintf(stderr, "[CoroRpcEchoCase1] done\n");

    // 等待网络库停止
    uint64_t used_time = 0;
    uint64_t old_time = 0;
    uint64_t run_mill_seconds = 24 * 60 * 60 * 1000;
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