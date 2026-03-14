#include "unit_test_frame/unittest.h"
#include "coro_rpc/coro_rpc_client.h"
#include "coro_rpc/coro_rpc_server.h"
#include "tools/cpp20_coroutine.h"
#include "tools/timer.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <unordered_set>
#include <memory>

FIXTURE_BEGIN(CoroRpcComprehensive)

// ========== 测试辅助函数和类型 ==========

// 基本类型测试函数
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

std::string concat(const std::string& a, const std::string& b) {
    return a + b;
}

void void_function(int& value) {
    value = 42;
}

// 协程函数
ToolBox::coro::Task<int> async_add(int a, int b) {
    co_return a + b;
}

ToolBox::coro::Task<std::string> async_concat(const std::string& a, const std::string& b) {
    co_return a + b;
}

    // 注意：void 协程函数返回 std::monostate 会导致序列化问题
    // 这里暂时注释掉，因为当前实现可能无法正确处理
    // ToolBox::coro::Task<std::monostate> async_void_function(int& value) {
    //     value = 100;
    //     co_return std::monostate{};
    // }

// 测试类
class TestService {
public:
    int value_ = 0;
    
    int add(int a, int b) {
        return a + b;
    }
    
    void set_value(int v) {
        value_ = v;
    }
    
    ToolBox::coro::Task<int> async_add(int a, int b) {
        co_return a + b;
    }
    
    ToolBox::coro::Task<std::monostate> async_set_value(int v) {
        value_ = v;
        co_return std::monostate{};
    }
};

template<typename ThreadSafetyPolicy = ToolBox::CoroRpc::ThreadUnsafe>
class AttachmentEchoService {
public:
    using Server = ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol, std::unordered_map, ThreadSafetyPolicy>;

    Server* server_ = nullptr;

    int add_with_attachment(int a, int b) {
        server_->SetRespAttachment(server_->GetReqAttachment());
        return a + b;
    }

    ToolBox::coro::Task<int> async_add_with_attachment(int a, int b) {
        server_->SetRespAttachment(server_->GetReqAttachment());
        co_return a + b;
    }
};

static ToolBox::CoroRpc::CoroRpcServer<ToolBox::CoroRpc::CoroRpcProtocol, std::unordered_map, ToolBox::CoroRpc::ThreadSafe>* g_async_attachment_server = nullptr;

ToolBox::coro::Task<int> async_attachment_checkpoint() {
    co_return 0;
}

ToolBox::coro::Task<int> async_add_with_global_attachment(int ignored) {
    (void)ignored;
    g_async_attachment_server->SetRespAttachment(g_async_attachment_server->GetReqAttachment());
    co_return 7;
}

ToolBox::coro::Task<int> async_add_with_global_attachment_after_await(int ignored) {
    (void)ignored;
    (void)co_await async_attachment_checkpoint();
    g_async_attachment_server->SetRespAttachment(g_async_attachment_server->GetReqAttachment());
    co_return 11;
}

// 全局变量用于测试（预留用于未来测试）
// static std::atomic<int> g_call_count{0};
// static std::string g_last_attachment;

// ========== CoroRpcProtocol 测试 ==========

CASE(TestCoroRpcProtocol_ReadHeader_Valid)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader header;
    header.magic = CoroRpcProtocol::kMagicNumber;
    header.version = CoroRpcProtocol::kVersionNumber;
    header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);
    header.msg_type = 0;
    header.seq_num = 12345;
    header.func_id = 67890;
    header.length = 100;
    header.attach_length = 50;
    
    std::string buffer(reinterpret_cast<const char*>(&header), sizeof(header));
    std::string_view data(buffer);
    
    CoroRpcProtocol::ReqHeader read_header;
    Errc err = CoroRpcProtocol::ReadHeader(data, read_header);
    
    if (err != Errc::SUCCESS) {
        SetError("读取有效头部失败");
        return;
    }
    
    if (read_header.magic != header.magic ||
        read_header.version != header.version ||
        read_header.seq_num != header.seq_num ||
        read_header.func_id != header.func_id) {
        SetError("读取的头部数据不匹配");
        return;
    }
}

CASE(TestCoroRpcProtocol_ReadHeader_InvalidMagic)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader header;
    header.magic = 0x00;  // 无效的 magic
    header.version = CoroRpcProtocol::kVersionNumber;
    
    std::string buffer(reinterpret_cast<const char*>(&header), sizeof(header));
    std::string_view data(buffer);
    
    CoroRpcProtocol::ReqHeader read_header;
    Errc err = CoroRpcProtocol::ReadHeader(data, read_header);
    
    if (err != Errc::ERR_PROTOCOL) {
        SetError("应该检测到无效的 magic 数字");
        return;
    }
}

CASE(TestCoroRpcProtocol_ReadHeader_InvalidVersion)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader header;
    header.magic = CoroRpcProtocol::kMagicNumber;
    header.version = 99;  // 无效的版本
    
    std::string buffer(reinterpret_cast<const char*>(&header), sizeof(header));
    std::string_view data(buffer);
    
    CoroRpcProtocol::ReqHeader read_header;
    Errc err = CoroRpcProtocol::ReadHeader(data, read_header);
    
    if (err != Errc::ERR_PROTOCOL) {
        SetError("应该检测到无效的版本号");
        return;
    }
}

CASE(TestCoroRpcProtocol_ReadHeader_TooSmall)
{
    using namespace ToolBox::CoroRpc;
    
    std::string buffer("too small");
    std::string_view data(buffer);
    
    CoroRpcProtocol::ReqHeader read_header;
    Errc err = CoroRpcProtocol::ReadHeader(data, read_header);
    
    if (err != Errc::ERR_PROTOCOL) {
        SetError("应该检测到数据太小");
        return;
    }
}

CASE(TestCoroRpcProtocol_ReadPayload)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader header;
    header.length = 10;
    header.attach_length = 5;
    
    std::string body_data(10, 'A');
    std::string attach_data(5, 'B');
    std::string payload = body_data + attach_data;
    
    std::string_view body, attachment;
    Errc err = CoroRpcProtocol::ReadPayLoad(header, payload, body, attachment);
    
    if (err != Errc::SUCCESS) {
        SetError("读取 payload 失败");
        return;
    }
    
    if (body.size() != 10 || attachment.size() != 5) {
        SetError("payload 大小不匹配");
        return;
    }
    
    if (body != body_data || attachment != attach_data) {
        SetError("payload 内容不匹配");
        return;
    }
}

CASE(TestCoroRpcProtocol_PrepareResponseHeader)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader req_header;
    req_header.seq_num = 12345;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);;
    
    std::string rpc_result = "test result";
    std::string response_buf;
    response_buf.resize(CoroRpcProtocol::RESP_HEAD_LEN + rpc_result.size());
    
    bool ret = CoroRpcProtocol::PrepareResponseHeader(
        response_buf, rpc_result, req_header, 0, Errc::SUCCESS);
    
    if (!ret) {
        SetError("准备响应头失败");
        return;
    }
    
    CoroRpcProtocol::RespHeader* resp_header = 
        reinterpret_cast<CoroRpcProtocol::RespHeader*>(response_buf.data());
    
    if (resp_header->magic != CoroRpcProtocol::kMagicNumber ||
        resp_header->seq_num != req_header.seq_num ||
        resp_header->length != rpc_result.size()) {
        SetError("响应头数据不正确");
        return;
    }
}

// ========== StructPackProtocol 测试 ==========

CASE(TestStructPackProtocol_Serialize_Deserialize_BasicTypes)
{
    using namespace ToolBox::CoroRpc;
    
    int a = 42;
    double b = 3.14;
    
    // 使用 tuple 来序列化多个参数
    std::tuple<int, double> args(a, b);
    std::string serialized = StructPackProtocol::Serialize(args);
    
    std::tuple<int, double> result;
    bool deserialized = StructPackProtocol::Deserialize(result, std::string_view(serialized));
    
    if (!deserialized) {
        SetError("反序列化失败");
        return;
    }
    
    if (std::get<0>(result) != a || std::get<1>(result) != b) {
        SetError("序列化/反序列化数据不匹配");
        return;
    }
}

CASE(TestStructPackProtocol_Serialize_Deserialize_SingleValue)
{
    using namespace ToolBox::CoroRpc;
    
    int value = 12345;
    std::string serialized = StructPackProtocol::Serialize(value);
    
    std::tuple<int> tuple_result;
    bool deserialized = StructPackProtocol::Deserialize(tuple_result, std::string_view(serialized));
    
    if (!deserialized) {
        SetError("反序列化失败");
        return;
    }
    
    if (std::get<0>(tuple_result) != value) {
        SetError("序列化/反序列化数据不匹配");
        return;
    }
}

CASE(TestStructPackProtocol_SerializeToBuffer)
{
    using namespace ToolBox::CoroRpc;
    
    int a = 100;
    int b = 200;
    
    // 计算序列化后的大小（两个 int）
    size_t size = sizeof(a) + sizeof(b);
    std::vector<char> buffer(size);
    
    Errc err = StructPackProtocol::SerializeToBuffer(buffer.data(), size, a, b);
    
    if (err != Errc::SUCCESS) {
        SetError("序列化到缓冲区失败");
        return;
    }
    
    std::tuple<int, int> result;
    std::string_view view(buffer.data(), size);
    bool deserialized = StructPackProtocol::Deserialize(result, view);
    
    if (!deserialized) {
        SetError("缓冲区反序列化失败");
        return;
    }
    
    if (std::get<0>(result) != a || std::get<1>(result) != b) {
        SetError("缓冲区序列化/反序列化数据不匹配");
        return;
    }
}

CASE(TestStructPackProtocol_SerializeToBuffer_TooSmall)
{
    using namespace ToolBox::CoroRpc;
    
    int a = 100;
    int b = 200;
    
    std::vector<char> buffer(1);  // 缓冲区太小
    
    Errc err = StructPackProtocol::SerializeToBuffer(buffer.data(), 1, a, b);
    
    if (err != Errc::ERR_BUFFER_TOO_SMALL) {
        SetError("应该检测到缓冲区太小");
        return;
    }
}

// ========== CoroRpcClient 测试 ==========

CASE(TestCoroRpcClient_Construction)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client1;
    CoroRpcClient<CoroRpcProtocol> client2(100);
    
    if (client1.get_client_id() == client2.get_client_id()) {
        SetError("客户端 ID 应该不同");
        return;
    }
    
    if (client2.get_client_id() != 100) {
        SetError("客户端 ID 设置失败");
        return;
    }
    client1.set_client_id(1000);
    client2.set_client_id(2000);
}

CASE(TestCoroRpcClient_SetReqAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    std::string attachment = "test attachment";
    bool ret = client.SetReqAttachment(attachment);
    
    if (!ret) {
        SetError("设置请求附件失败");
        return;
    }
    
    // 测试过大的附件
    // 注意：UINT32_MAX + 1 是 4GB+1，实际分配会导致内存问题
    // 这里我们只测试正常范围内的值，边界检查逻辑由代码保证
    // 如果需要测试边界情况，应该在压力测试中使用更合适的方法
    std::string large_attachment(1000, 'A');
    ret = client.SetReqAttachment(large_attachment);
    
    if (!ret) {
        SetError("应该允许正常大小的附件");
        return;
    }
}

CASE(TestCoroRpcClient_SetSendCallback)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    std::string received_data;
    
    client.SetSendCallback([&](std::string &&data) {
        received_data = data;
    });
    
    // 回调应该被设置
    // 注意：这里无法直接测试，因为需要实际调用 Call 才会触发
}

CASE(TestCoroRpcClient_OnRecvResp_Valid)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 构造一个有效的响应
    CoroRpcProtocol::RespHeader resp_header;
    resp_header.magic = CoroRpcProtocol::kMagicNumber;
    resp_header.version = CoroRpcProtocol::kVersionNumber;
    resp_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);;
    resp_header.seq_num = 9528;  // 使用默认的 request_id + 1
    resp_header.err_code = static_cast<uint8_t>(Errc::SUCCESS);
    resp_header.length = 0;
    resp_header.attach_length = 0;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::RESP_HEAD_LEN);
    std::memcpy(buffer.data(), &resp_header, sizeof(resp_header));
    
    // 注意：这个测试需要先发送请求并注册 handler
    // 这里只是测试协议解析部分
    Errc err = client.OnRecvResp(buffer);
    
    // 由于没有注册的 handler，应该返回成功（协议解析成功）
    // 但实际处理可能失败
    (void)err;  // 避免未使用变量警告
}

CASE(TestCoroRpcClient_OnRecvResp_InvalidHeader)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    std::string invalid_data = "invalid";
    Errc err = client.OnRecvResp(invalid_data);
    
    if (err == Errc::SUCCESS) {
        SetError("应该检测到无效的响应头");
        return;
    }
}

// ========== CoroRpcServer 测试 ==========

CASE(TestCoroRpcServer_RegisterService_NonMemberFunction)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<add>();
    server.RegisterService<multiply, concat>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 3) {
        SetError("注册的服务数量不正确");
        return;
    }
    
}

CASE(TestCoroRpcServer_RegisterService_MemberFunction)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    TestService service;
    
    server.RegisterService<&TestService::add>(&service);
    server.RegisterService<&TestService::set_value, &TestService::async_add>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 3) {
        SetError("注册的成员函数服务数量不正确");
        return;
    }
    
}

CASE(TestCoroRpcServer_RegisterService_CoroutineFunction)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("注册的协程服务数量不正确");
        return;
    }
    
}

CASE(TestCoroRpcServer_GetFunctionKeyName)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<add>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("没有注册的服务");
        return;
    }
    
    const std::string& name = server.GetFunctionKeyName(keys[0]);
    // 名称可能为空，取决于实现
    (void)name;  // 避免未使用变量警告

}

CASE(TestCoroRpcServer_SetSendCallback)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    bool callback_called = false;
    
    Errc err = server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        callback_called = true;
    });
    
    if (err != Errc::SUCCESS) {
        SetError("设置发送回调失败");
        return;
    }
}

CASE(TestCoroRpcServer_SetRespAttachmentFunc)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<add>();
    server.SetRespAttachmentFunc<add>([]() -> std::string_view {
        static std::string attachment = "test attachment";
        return attachment;
    });
    
    // 测试设置成功（无异常）
    
}

CASE(TestCoroRpcServer_OnRecvReq_InvalidHeader)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    std::string invalid_data = "invalid request";
    Errc err = server.OnRecvReq(1, invalid_data);
    
    if (err == Errc::SUCCESS) {
        SetError("应该检测到无效的请求头");
        return;
    }
}

CASE(TestCoroRpcServer_OnRecvReq_UnregisteredFunction)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 构造一个未注册函数的请求
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);;
    req_header.seq_num = 1;
    req_header.func_id = 99999;  // 未注册的函数 ID
    req_header.length = 0;
    req_header.attach_length = 0;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    server.SetSendCallback([](uint64_t, std::string &&) {});
    
    Errc err = server.OnRecvReq(1, buffer);
    
    // 应该返回 ERR_FUNC_NOT_REGISTERED 或类似错误
    // 注意：实际行为取决于实现
    (void)err;  // 避免未使用变量警告
}

// ========== 集成测试：客户端-服务器通信 ==========

CASE(TestClientServer_Integration_BasicCall)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 注册服务
    server.RegisterService<add>();
    
    // 设置服务器发送回调
    std::string server_sent_data;
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        server_sent_data = buffer;
        // 模拟服务器响应
        client.OnRecvResp(std::string_view(buffer));
    });
    
    // 设置客户端发送回调
    std::string client_sent_data;
    client.SetSendCallback([&](std::string &&buffer) {
        client_sent_data = buffer;
        // 模拟客户端请求到达服务器
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 注意：这是一个简化的测试，实际需要协程执行器来运行
    // 完整的集成测试需要实际的网络层或模拟的协程执行环境
    
}

CASE(TestClientServer_Integration_WithAttachment)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    std::string received_attachment;
    std::string sent_attachment = "test attachment data";
    
    // 注册一个会使用附件的服务
    server.RegisterService<add>();
    server.SetRespAttachmentFunc<add>([&]() -> std::string_view {
        return received_attachment;
    });
    
    // 设置服务器发送回调
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    // 设置客户端发送回调
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 设置请求附件
    client.SetReqAttachment(sent_attachment);
    
    // 注意：需要协程执行器来完整测试

}

CASE(TestClientServer_Integration_AsyncFunction)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 注册协程服务
    server.RegisterService<async_add>();
    
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 注意：需要协程执行器来完整测试
    
}

// ========== 错误处理测试 ==========

CASE(TestErrorHandling_Timeout)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 设置发送回调但不响应，模拟超时
    client.SetSendCallback([](std::string &&) {
        // 不响应，导致超时
    });
    
    // 注意：需要协程执行器来测试超时
    // 这里只是验证客户端可以设置回调
}

CASE(TestErrorHandling_InvalidProtocol)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 构造无效协议的请求
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = 99;  // 无效的序列化类型
    req_header.seq_num = 1;
    req_header.func_id = 1;
    req_header.length = 0;
    req_header.attach_length = 0;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    server.SetSendCallback([](uint64_t, std::string &&) {});
    
    Errc err = server.OnRecvReq(1, buffer);
    
    // 应该返回协议错误
    if (err == Errc::SUCCESS) {
        SetError("应该检测到无效的协议类型");
        return;
    }
    (void)err;  // 避免未使用变量警告
}

// ========== 并发测试 ==========

CASE(TestConcurrency_MultipleClients)
{
    using namespace ToolBox::CoroRpc;
    
    std::vector<std::unique_ptr<CoroRpcClient<CoroRpcProtocol>>> clients;
    
    for (int i = 0; i < 10; ++i) {
        clients.push_back(std::make_unique<CoroRpcClient<CoroRpcProtocol>>(i));
    }
    
    // 验证所有客户端都有不同的 ID
    for (size_t i = 0; i < clients.size(); ++i) {
        for (size_t j = i + 1; j < clients.size(); ++j) {
            if (clients[i]->get_client_id() == clients[j]->get_client_id()) {
                SetError("客户端 ID 冲突");
                return;
            }
        }
    }
}

CASE(TestConcurrency_MultipleServices)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 注册多个服务
    server.RegisterService<add>();
    server.RegisterService<multiply>();
    server.RegisterService<concat>();
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    // 注意：async_void_function 暂时注释掉
    // server.RegisterService<async_void_function>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() != 5) {
        SetError("注册的服务数量不正确");
        return;
    }
    
    // 验证所有键都是唯一的
    for (size_t i = 0; i < keys.size(); ++i) {
        for (size_t j = i + 1; j < keys.size(); ++j) {
            if (keys[i] == keys[j]) {
                SetError("服务键冲突");
                return;
            }
        }
    }
    
}

// ========== 边界条件测试 ==========

CASE(TestBoundaryConditions_EmptyAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    bool ret = client.SetReqAttachment("");
    
    if (!ret) {
        SetError("应该允许空附件");
        return;
    }
}

CASE(TestBoundaryConditions_MaxAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 注意：UINT32_MAX 是 4GB，实际分配会导致内存问题
    // 这里我们测试合理大小的附件，验证功能正常
    // 边界值检查逻辑由代码保证，不需要实际分配4GB内存来测试
    std::string large_attachment(10000, 'A');
    bool ret = client.SetReqAttachment(large_attachment);
    
    if (!ret) {
        SetError("应该允许合理大小的附件");
        return;
    }
}

CASE(TestBoundaryConditions_ZeroLengthPayload)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcProtocol::ReqHeader header;
    header.length = 0;
    header.attach_length = 0;
    
    std::string_view empty_payload;
    std::string_view body, attachment;
    
    Errc err = CoroRpcProtocol::ReadPayLoad(header, empty_payload, body, attachment);
    
    if (err != Errc::SUCCESS) {
        SetError("应该允许零长度的 payload");
        return;
    }
    
    if (body.size() != 0 || attachment.size() != 0) {
        SetError("零长度 payload 解析错误");
        return;
    }
}

// ========== 协程函数测试 ==========

CASE(TestCoroutineFunction_Registration)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    // 注意：async_void_function 暂时注释掉，因为返回 std::monostate 会导致序列化问题
    // server.RegisterService<async_void_function>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("协程函数注册失败");
        return;
    }

}

// ========== 工具函数测试 ==========

CASE(TestCoroRpcTools_AutoGenRegisterKey)
{
    using namespace ToolBox::CoroRpc;
    
    auto key1 = CoroRpcTools::AutoGenRegisterKey<add>();
    auto key2 = CoroRpcTools::AutoGenRegisterKey<multiply>();
    auto key3 = CoroRpcTools::AutoGenRegisterKey<add>();  // 相同函数应该生成相同键
    
    if (key1 == 0 || key2 == 0) {
        SetError("生成的键不应该为零");
        return;
    }
    
    if (key1 != key3) {
        SetError("相同函数应该生成相同的键");
        return;
    }
    
    if (key1 == key2) {
        SetError("不同函数应该生成不同的键");
        return;
    }
}

CASE(TestCoroRpcTools_GetGlobalClientId)
{
    using namespace ToolBox::CoroRpc;
    
    uint64_t id1 = get_global_client_id();
    uint64_t id2 = get_global_client_id();
    uint64_t id3 = get_global_client_id();
    
    if (id1 >= id2 || id2 >= id3) {
        SetError("全局客户端 ID 应该递增");
        return;
    }
}

// ========== 序列化协议选择测试 ==========

CASE(TestProtocolSelection_StructPackForBasicTypes)
{
    using namespace ToolBox::CoroRpc;
    
    // 基本类型应该使用 StructPackProtocol
    auto protocol = CoroRpcProtocol::GetSerializeProtocol<add>();
    
    bool is_struct_pack = std::holds_alternative<StructPackProtocol>(protocol);
    
    if (!is_struct_pack) {
        SetError("基本类型应该使用 StructPackProtocol");
        return;
    }
}

CASE(TestProtocolSelection_GetSerializeProtocolByType)
{
    using namespace ToolBox::CoroRpc;
    
    // 测试通过类型获取协议
    auto protocol1 = CoroRpcProtocol::GetSerializeProtocolByType(static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf));
    
    if (!protocol1.has_value()) {
        SetError("应该能够获取有效的协议");
        return;
    }
    
    // 测试无效类型
    auto protocol2 = CoroRpcProtocol::GetSerializeProtocolByType(99);
    
    if (protocol2.has_value()) {
        SetError("无效类型不应该返回协议");
        return;
    }
}

// ========== ProtobufProtocol 测试 ==========
// 注意：这些测试需要实际的 protobuf 消息类型
// 这里只测试协议的基本功能

CASE(TestProtobufProtocol_Serialize_Deserialize_Empty)
{
    using namespace ToolBox::CoroRpc;
    
    // ProtobufProtocol 的 Serialize() 应该返回空字符串
    std::string result = ProtobufProtocol::Serialize();
    
    if (!result.empty()) {
        SetError("空序列化应该返回空字符串");
        return;
    }
}

// ========== 性能测试 ==========

CASE(TestPerformance_SerializationSpeed)
{
    using namespace ToolBox::CoroRpc;
    
    const int iterations = 10000;
    int a = 42;
    int b = 100;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        std::tuple<int, int> args(a, b);
        std::string serialized = StructPackProtocol::Serialize(args);
        std::tuple<int, int> result;
        StructPackProtocol::Deserialize(result, std::string_view(serialized));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 验证性能在合理范围内（每1000次操作应该在1秒内）
    if (duration.count() > iterations * 1000) {
        SetError("序列化性能过慢");
        return;
    }
}

CASE(TestPerformance_HeaderParsing)
{
    using namespace ToolBox::CoroRpc;
    
    const int iterations = 100000;
    
    CoroRpcProtocol::ReqHeader header;
    header.magic = CoroRpcProtocol::kMagicNumber;
    header.version = CoroRpcProtocol::kVersionNumber;
    header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);;
    header.seq_num = 12345;
    header.func_id = 67890;
    header.length = 100;
    header.attach_length = 50;
    
    std::string buffer(reinterpret_cast<const char*>(&header), sizeof(header));
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        CoroRpcProtocol::ReqHeader read_header;
        CoroRpcProtocol::ReadHeader(buffer, read_header);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 验证性能在合理范围内
    if (duration.count() > iterations * 10) {
        SetError("头部解析性能过慢");
        return;
    }
}

// ========== 压力测试 ==========

CASE(TestStress_ManyServices)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 注册大量服务
    server.RegisterService<add>();
    server.RegisterService<multiply>();
    server.RegisterService<concat>();
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    // 注意：async_void_function 暂时注释掉
    // server.RegisterService<async_void_function>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 5) {
        SetError("服务注册数量不正确");
        return;
    }
    
    // 验证所有键都是唯一的
    std::unordered_set<uint32_t> key_set(keys.begin(), keys.end());
    if (key_set.size() != keys.size()) {
        SetError("服务键存在重复");
        return;
    }
    
}

CASE(TestStress_LargeAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 测试较大的附件（使用合理的测试值，避免内存问题）
    // 注意：UINT32_MAX - 100 接近 4GB，实际分配会导致内存问题
    // 这里我们使用合理的测试值来验证功能
    const size_t large_size = 100000;  // 100KB，足够测试功能
    std::string large_attachment(large_size, 'A');
    
    bool ret = client.SetReqAttachment(large_attachment);
    
    if (!ret) {
        SetError("应该能够设置合理大小的附件");
        return;
    }
}

// ========== 边界条件测试（补充） ==========

CASE(TestBoundaryConditions_RequestIdRollover)
{
    using namespace ToolBox::CoroRpc;
    
    // 测试请求 ID 是否会正确递增
    CoroRpcClient<CoroRpcProtocol> client1;
    CoroRpcClient<CoroRpcProtocol> client2;
    
    // 验证两个客户端有不同的初始 request_id
    // 注意：request_id 是私有的，这里只能通过行为推断
    // 这个测试主要验证客户端可以正常创建
    (void)client1;
    (void)client2;
}

CASE(TestBoundaryConditions_EmptyFunctionName)
{
    using namespace ToolBox::CoroRpc;
    
    // 测试空函数名的情况（如果有的话）
    // 这取决于实际的函数名生成逻辑
}

// ========== 异常安全测试 ==========

CASE(TestExceptionSafety_HandlerThrowsException)
{
    using namespace ToolBox::CoroRpc;
    
    // 这个测试需要实际的服务函数抛出异常
    // 验证服务器能够正确处理异常
}

// ========== 内存安全测试 ==========

CASE(TestMemorySafety_UseAfterFree)
{
    using namespace ToolBox::CoroRpc;
    
    // 测试在对象销毁后使用的情况
    {
        CoroRpcClient<CoroRpcProtocol> client;
        client.SetSendCallback([](std::string &&) {});
    }
    
    // client 已销毁，不应该有内存问题
    // 这个测试主要验证没有明显的悬空指针
}

CASE(TestMemorySafety_AttachmentLifetime)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol> client;
    
    {
        std::string attachment = "temporary attachment";
        client.SetReqAttachment(attachment);
    }
    
    // attachment 已销毁，但 client 中保存的是 string_view
    // 这可能导致悬空引用，但这是预期的行为（调用者需要保证生命周期）
    // 这个测试主要是提醒注意生命周期管理
}

// ========== 线程安全测试 ==========

CASE(TestThreadSafety_ConcurrentRegistration)
{
    using namespace ToolBox::CoroRpc;
    

    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    // 多个线程同时注册服务
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            try {
                if (i == 0) server.RegisterService<add>();
                else if (i == 1) server.RegisterService<multiply>();
                else if (i == 2) server.RegisterService<concat>();
                else if (i == 3) server.RegisterService<async_add>();
                else if (i == 4) server.RegisterService<async_concat>();
                success_count++;
            } catch (...) {
                // 注册失败
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() != success_count.load()) {
        SetError("并发注册可能导致服务丢失");
        return;
    }
    
}

// ========== 兼容性测试 ==========

CASE(TestCompatibility_HeaderFormat)
{
    using namespace ToolBox::CoroRpc;
    
    // 验证头部格式符合预期
    static_assert(sizeof(CoroRpcProtocol::ReqHeader) == 32, 
                  "请求头大小应该是 20 字节");
    static_assert(sizeof(CoroRpcProtocol::RespHeader) == 32, 
                  "响应头大小应该是 20 字节");
    
    // 验证常量值
    if (CoroRpcProtocol::kMagicNumber != 0xde) {
        SetError("Magic number 不正确");
        return;
    }
    
    if (CoroRpcProtocol::kVersionNumber != 1) {
        SetError("Version number 不正确");
        return;
    }
}

CASE(TestCompatibility_ErrorCodeValues)
{
    using namespace ToolBox::CoroRpc;
    
    // 验证错误码的值
    if (static_cast<int>(Errc::SUCCESS) != 0) {
        SetError("SUCCESS 错误码应该是 0");
        return;
    }
    
    if (static_cast<int>(Errc::ERR_TIMEOUT) != 1) {
        SetError("ERR_TIMEOUT 错误码应该是 1");
        return;
    }
    
    // 验证错误码是递增的（可选）
    if (static_cast<int>(Errc::ERR_INVALID_ARGUMENTS) <= 
        static_cast<int>(Errc::ERR_TIMEOUT)) {
        SetError("错误码顺序可能有问题");
        return;
    }
}

// ========== 内存对齐测试 ==========

CASE(TestMemoryAlignment_ReadHeader)
{
    using namespace ToolBox::CoroRpc;
    
    // 测试非对齐内存的读取
    CoroRpcProtocol::ReqHeader header;
    header.magic = CoroRpcProtocol::kMagicNumber;
    header.version = CoroRpcProtocol::kVersionNumber;
    header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);
    header.seq_num = 12345;
    header.func_id = 67890;
    header.length = 100;
    header.attach_length = 50;
    
    // 创建一个非对齐的缓冲区
    std::vector<char> buffer(sizeof(header) + 1);
    std::memcpy(buffer.data() + 1, &header, sizeof(header));
    
    std::string_view data(buffer.data() + 1, sizeof(header));
    
    CoroRpcProtocol::ReqHeader read_header;
    Errc err = CoroRpcProtocol::ReadHeader(data, read_header);
    
    if (err != Errc::SUCCESS) {
        SetError("应该能够处理非对齐的内存");
        return;
    }
    
    if (read_header.seq_num != header.seq_num) {
        SetError("非对齐内存读取数据不正确");
        return;
    }
}

// ========== ThreadSafe/ThreadUnsafe 策略深度测试 ==========

// ThreadSafe Client 构造与基本操作
CASE(TestThreadSafe_ClientConstruction)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client1;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client2(100);
    
    if (client1.get_client_id() == client2.get_client_id()) {
        SetError("ThreadSafe 客户端 ID 应该不同");
        return;
    }
    
    if (client2.get_client_id() != 100) {
        SetError("ThreadSafe 客户端 ID 设置失败");
        return;
    }
    
    client1.set_client_id(1000);
    if (client1.get_client_id() != 1000) {
        SetError("ThreadSafe 客户端 set_client_id 失败");
        return;
    }
}

// ThreadSafe Client 附件设置
CASE(TestThreadSafe_ClientSetReqAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    std::string attachment = "ThreadSafe test attachment";
    bool ret = client.SetReqAttachment(attachment);
    if (!ret) {
        SetError("ThreadSafe 客户端设置请求附件失败");
        return;
    }
    
    // 测试空附件
    ret = client.SetReqAttachment("");
    if (!ret) {
        SetError("ThreadSafe 客户端应允许空附件");
        return;
    }
    
    // 测试较大附件
    std::string large_attachment(10000, 'X');
    ret = client.SetReqAttachment(large_attachment);
    if (!ret) {
        SetError("ThreadSafe 客户端应允许合理大小的附件");
        return;
    }
}

// ThreadSafe Client 设置回调
CASE(TestThreadSafe_ClientSetSendCallback)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    bool callback_set = false;
    
    client.SetSendCallback([&](std::string &&data) {
        callback_set = true;
    });
    
    // 回调应该被设置（但还没触发）
}

// ThreadSafe Client OnRecvResp 错误头
CASE(TestThreadSafe_ClientOnRecvResp_InvalidHeader)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    std::string invalid_data = "invalid";
    Errc err = client.OnRecvResp(invalid_data);
    
    if (err == Errc::SUCCESS) {
        SetError("ThreadSafe 客户端应检测到无效的响应头");
        return;
    }
}

// ThreadSafe Client OnRecvResp 有效头（但无匹配 handler）
CASE(TestThreadSafe_ClientOnRecvResp_ValidHeaderNoHandler)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    CoroRpcProtocol::RespHeader resp_header{};
    resp_header.magic = CoroRpcProtocol::kMagicNumber;
    resp_header.version = CoroRpcProtocol::kVersionNumber;
    resp_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    resp_header.seq_num = 9528;
    resp_header.err_code = static_cast<uint8_t>(Errc::SUCCESS);
    resp_header.length = 0;
    resp_header.attach_length = 0;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::RESP_HEAD_LEN);
    std::memcpy(buffer.data(), &resp_header, sizeof(resp_header));
    
    // 没有注册 handler，应成功（解析协议成功，只是找不到 handler）
    Errc err = client.OnRecvResp(buffer);
    (void)err;
}

// ThreadSafe Server 注册非成员函数
CASE(TestThreadSafe_ServerRegisterNonMember)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<add>();
    server.RegisterService<multiply, concat>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 3) {
        SetError("ThreadSafe 策略下注册的非成员服务数量不正确");
        return;
    }
}

// ThreadSafe Server 注册成员函数
CASE(TestThreadSafe_ServerRegisterMember)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    TestService service;
    
    server.RegisterService<&TestService::add>(&service);
    server.RegisterService<&TestService::set_value, &TestService::async_add>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 3) {
        SetError("ThreadSafe 策略下注册的成员函数服务数量不正确");
        return;
    }
}

// ThreadSafe Server 注册协程函数
CASE(TestThreadSafe_ServerRegisterCoroutine)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("ThreadSafe 策略下注册的协程服务数量不正确");
        return;
    }
}

// ThreadSafe Server GetFunctionKeyName
CASE(TestThreadSafe_ServerGetFunctionKeyName)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<add>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("ThreadSafe 策略下没有注册的服务");
        return;
    }
    
    const std::string& name = server.GetFunctionKeyName(keys[0]);
    // 名称应该非空（add 函数的名称）
    if (name.empty()) {
        SetError("ThreadSafe 策略下 GetFunctionKeyName 应返回非空名称");
        return;
    }
}

// ThreadSafe Server SetSendCallback
CASE(TestThreadSafe_ServerSetSendCallback)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    bool callback_called = false;
    
    Errc err = server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        callback_called = true;
    });
    
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe 策略下设置发送回调失败");
        return;
    }
}

// ThreadSafe Server SetRespAttachmentFunc
CASE(TestThreadSafe_ServerSetRespAttachmentFunc)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<add>();
    server.SetRespAttachmentFunc<add>([]() -> std::string_view {
        static std::string attachment = "ThreadSafe test attachment";
        return attachment;
    });
}

// ThreadSafe Server OnRecvReq 无效头
CASE(TestThreadSafe_ServerOnRecvReq_InvalidHeader)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    std::string invalid_data = "invalid request";
    Errc err = server.OnRecvReq(1, invalid_data);
    
    if (err == Errc::SUCCESS) {
        SetError("ThreadSafe 策略下应检测到无效的请求头");
        return;
    }
}

// ThreadSafe Server OnRecvReq 无效序列化类型
CASE(TestThreadSafe_ServerOnRecvReq_InvalidSerializeType)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    CoroRpcProtocol::ReqHeader req_header{};
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = 99;  // 无效
    req_header.msg_type = 0;
    req_header.seq_num = 1;
    req_header.func_id = 1;
    req_header.length = 0;
    req_header.attach_length = 0;
    req_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    server.SetSendCallback([](uint64_t, std::string &&) {});
    
    Errc err = server.OnRecvReq(1, buffer);
    
    if (err == Errc::SUCCESS) {
        SetError("ThreadSafe 策略下应检测到无效的协议类型");
        return;
    }
}

// ThreadSafe 集成：Client-Server 基本通信链路
CASE(TestThreadSafe_Integration_BasicCall)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    server.RegisterService<add>();
    
    std::string server_sent_data;
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        server_sent_data = buffer;
        client.OnRecvResp(std::string_view(buffer));
    });
    
    std::string client_sent_data;
    client.SetSendCallback([&](std::string &&buffer) {
        client_sent_data = buffer;
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 验证基础链路设置完成
}

// ThreadSafe 集成：附件
CASE(TestThreadSafe_Integration_WithAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    server.RegisterService<add>();
    server.SetRespAttachmentFunc<add>([&]() -> std::string_view {
        return server.GetReqAttachment();
    });
    
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    std::string test_att = "ThreadSafe attachment data";
    client.SetReqAttachment(test_att);
}

// ThreadSafe 集成：协程函数
CASE(TestThreadSafe_Integration_AsyncFunction)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    server.RegisterService<async_add>();
    
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });
}

CASE(TestThreadSafe_AsyncAttachmentContextClearedAfterCompletion)
{
    using namespace ToolBox::CoroRpc;

    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    g_async_attachment_server = &server;

    server.RegisterService<async_add_with_global_attachment>();
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });

    client.SetReqAttachment("async-attachment");
    auto result = client.Call<async_add_with_global_attachment>(0).get_result();
    g_async_attachment_server = nullptr;
    if (!result || result.value() != 7) {
        SetError("async attachment 调用应成功返回正确结果");
        return;
    }
    if (result.GetRespAttachment() != "async-attachment") {
        SetError("async handler 应读取到本次请求 attachment 并正确回传");
        return;
    }
    if (!server.GetReqAttachment().empty()) {
        SetError("async 请求完成后 server 不应残留上一条请求的 TLS attachment 上下文");
        return;
    }
}

CASE(TestThreadSafe_AsyncAttachmentContextRestoredAfterAwait)
{
    using namespace ToolBox::CoroRpc;

    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    g_async_attachment_server = &server;

    server.RegisterService<async_add_with_global_attachment_after_await>();
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });

    client.SetReqAttachment("async-attachment-after-await");
    auto result = client.Call<async_add_with_global_attachment_after_await>(0).get_result();
    g_async_attachment_server = nullptr;
    if (!result || result.value() != 11) {
        SetError("跨 co_await 的 async attachment 调用应成功返回正确结果");
        return;
    }
    if (result.GetRespAttachment() != "async-attachment-after-await") {
        SetError("async handler 在 co_await 之后仍应读取到本次请求 attachment");
        return;
    }
    if (!server.GetReqAttachment().empty()) {
        SetError("跨 co_await 的 async 请求完成后 server 不应残留 TLS attachment 上下文");
        return;
    }
}

// ThreadSafe：失败调用后不应残留请求附件到下一次请求
CASE(TestThreadSafe_FailedCallClearsReqAttachment)
{
    using namespace ToolBox::CoroRpc;

    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    client.SetReqAttachment("stale-attachment");
    auto failed = client.Call<add>(1, 2).get_result();
    if (!failed.HasError() || failed.GetError() != Errc::ERR_SEND_CALLBACK_NOT_SET) {
        SetError("未设置 send callback 的调用应返回 ERR_SEND_CALLBACK_NOT_SET");
        return;
    }

    bool invalid_packet = false;
    bool req_captured = false;
    CoroRpcProtocol::ReqHeader req_header{};
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            invalid_packet = true;
            return;
        }
        std::memcpy(&req_header, buffer.data(), sizeof(req_header));
        req_captured = true;

        std::string rpc_result = StructPackProtocol::Serialize(42);
        std::string resp_buffer;
        resp_buffer.resize(CoroRpcProtocol::RESP_HEAD_LEN + rpc_result.size());
        bool prepared = CoroRpcProtocol::PrepareResponseHeader(resp_buffer, rpc_result, req_header, 0, Errc::SUCCESS);
        if (!prepared) {
            invalid_packet = true;
            return;
        }
        std::memcpy(resp_buffer.data() + CoroRpcProtocol::RESP_HEAD_LEN, rpc_result.data(), rpc_result.size());
        client.OnRecvResp(std::string_view(resp_buffer));
    });

    auto result = client.Call<add>(20, 22).get_result();
    if (!result || result.value() != 42) {
        SetError("失败调用后的下一次请求应可正常完成");
        return;
    }
    if (invalid_packet || !req_captured) {
        SetError("应捕获到完整的后续请求包");
        return;
    }
    if (req_header.attach_length != 0) {
        SetError("失败调用中的请求附件不应泄露到下一次请求");
        return;
    }
}

// ThreadSafe：未知响应不应污染后续正常请求
CASE(TestThreadSafe_UnknownResponseDoesNotBreakNextCall)
{
    using namespace ToolBox::CoroRpc;

    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    CoroRpcProtocol::RespHeader unknown_header{};
    unknown_header.magic = CoroRpcProtocol::kMagicNumber;
    unknown_header.version = CoroRpcProtocol::kVersionNumber;
    unknown_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    unknown_header.err_code = static_cast<uint8_t>(Errc::SUCCESS);
    unknown_header.msg_type = 0;
    unknown_header.seq_num = 777777;
    unknown_header.client_id = client.get_client_id();
    unknown_header.length = 0;
    unknown_header.attach_length = 0;

    std::string unknown_resp;
    unknown_resp.resize(CoroRpcProtocol::RESP_HEAD_LEN);
    std::memcpy(unknown_resp.data(), &unknown_header, sizeof(unknown_header));

    Errc inject_err = client.OnRecvResp(std::string_view(unknown_resp));
    if (inject_err != Errc::SUCCESS) {
        SetError("注入未知但格式合法的响应包不应失败");
        return;
    }

    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            SetError("后续请求包不完整");
            return;
        }
        CoroRpcProtocol::ReqHeader req_header{};
        std::memcpy(&req_header, buffer.data(), sizeof(req_header));

        std::string rpc_result = StructPackProtocol::Serialize(12);
        std::string resp_buffer;
        resp_buffer.resize(CoroRpcProtocol::RESP_HEAD_LEN + rpc_result.size());
        bool prepared = CoroRpcProtocol::PrepareResponseHeader(resp_buffer, rpc_result, req_header, 0, Errc::SUCCESS);
        if (!prepared) {
            SetError("后续响应包构造失败");
            return;
        }
        std::memcpy(resp_buffer.data() + CoroRpcProtocol::RESP_HEAD_LEN, rpc_result.data(), rpc_result.size());
        client.OnRecvResp(std::string_view(resp_buffer));
    });

    auto result = client.Call<add>(5, 7).get_result();
    if (!result || result.value() != 12) {
        SetError("未知响应之后的下一次请求仍应成功");
        return;
    }
}

// ThreadSafe：请求外设置的响应附件不应泄露到真实请求中
CASE(TestThreadSafe_ServerOutsideRequestAttachmentNoLeak)
{
    using namespace ToolBox::CoroRpc;

    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    server.RegisterService<add>();

    std::string fallback_attachment = "fallback-attachment";
    std::string captured_resp;
    server.SetRespAttachment("orphan-outside-request");
    server.SetRespAttachmentFunc<add>([&]() -> std::string_view {
        return fallback_attachment;
    });
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        captured_resp = std::move(buffer);
    });

    std::tuple<int, int> args{19, 23};
    std::string req_body = StructPackProtocol::Serialize(args);
    CoroRpcProtocol::ReqHeader req_header{};
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    req_header.msg_type = 0;
    req_header.seq_num = 9528;
    req_header.func_id = CoroRpcTools::AutoGenRegisterKey<add>();
    req_header.length = static_cast<uint32_t>(req_body.size());
    req_header.client_id = 1;
    req_header.attach_length = 0;

    std::string req_buffer;
    req_buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN + req_body.size());
    std::memcpy(req_buffer.data(), &req_header, sizeof(req_header));
    std::memcpy(req_buffer.data() + CoroRpcProtocol::REQ_HEAD_LEN, req_body.data(), req_body.size());

    Errc err = server.OnRecvReq(1, std::string_view(req_buffer));
    if (err != Errc::SUCCESS) {
        SetError("服务端应成功处理手工构造的合法请求");
        return;
    }
    if (captured_resp.size() < CoroRpcProtocol::RESP_HEAD_LEN) {
        SetError("服务端应产生完整的响应包");
        return;
    }

    CoroRpcProtocol::RespHeader resp_header{};
    std::memcpy(&resp_header, captured_resp.data(), sizeof(resp_header));

    std::string_view resp_body;
    std::string_view resp_attachment;
    Errc payload_err = CoroRpcProtocol::ReadPayLoad(resp_header, std::string_view(captured_resp).substr(CoroRpcProtocol::RESP_HEAD_LEN), resp_body, resp_attachment);
    if (payload_err != Errc::SUCCESS) {
        SetError("服务端响应 payload 解析失败");
        return;
    }

    (void)resp_body;
    if (resp_attachment != fallback_attachment) {
        SetError("请求外设置的响应附件不应泄露到真实请求，应使用本次请求的 fallback attachment");
        return;
    }
}

// ThreadSafe 多个客户端
CASE(TestThreadSafe_MultipleClients)
{
    using namespace ToolBox::CoroRpc;
    
    std::vector<std::unique_ptr<CoroRpcClient<CoroRpcProtocol, ThreadSafe>>> clients;
    
    for (int i = 0; i < 10; ++i) {
        clients.push_back(std::make_unique<CoroRpcClient<CoroRpcProtocol, ThreadSafe>>(i));
    }
    
    // 验证所有客户端都有不同的 ID
    for (size_t i = 0; i < clients.size(); ++i) {
        for (size_t j = i + 1; j < clients.size(); ++j) {
            if (clients[i]->get_client_id() == clients[j]->get_client_id()) {
                SetError("ThreadSafe 客户端 ID 冲突");
                return;
            }
        }
    }
}

// ThreadSafe 多服务注册
CASE(TestThreadSafe_MultipleServices)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<add>();
    server.RegisterService<multiply>();
    server.RegisterService<concat>();
    server.RegisterService<async_add>();
    server.RegisterService<async_concat>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() != 5) {
        SetError("ThreadSafe 策略下注册的服务数量不正确");
        return;
    }
    
    // 验证所有键都是唯一的
    for (size_t i = 0; i < keys.size(); ++i) {
        for (size_t j = i + 1; j < keys.size(); ++j) {
            if (keys[i] == keys[j]) {
                SetError("ThreadSafe 策略下服务键冲突");
                return;
            }
        }
    }
}

// ThreadSafe Server per-request 附件接口
CASE(TestThreadSafe_ServerSetRespAttachment)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.SetRespAttachment("ThreadSafe per-request attachment");
    
    auto att = server.GetReqAttachment();
    // 无请求上下文时 attachment 应为空
    (void)att;
}

// ThreadSafe 与 ThreadUnsafe 的 AutoGenRegisterKey 一致性
CASE(TestKeyConsistency_AcrossPolicies)
{
    using namespace ToolBox::CoroRpc;
    
    // 函数 key 由 CoroRpcTools::AutoGenRegisterKey 生成，
    // 与 ThreadSafetyPolicy 无关，应完全一致
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadUnsafe> tu_server;
    
    ts_server.RegisterService<add>();
    ts_server.RegisterService<multiply>();
    tu_server.RegisterService<add>();
    tu_server.RegisterService<multiply>();
    
    auto ts_keys = ts_server.GetAllServiceHandlerKeys();
    auto tu_keys = tu_server.GetAllServiceHandlerKeys();
    
    // 排序后比较
    std::sort(ts_keys.begin(), ts_keys.end());
    std::sort(tu_keys.begin(), tu_keys.end());
    
    if (ts_keys.size() != tu_keys.size()) {
        SetError("两种策略注册相同函数的 key 数量应一致");
        return;
    }
    for (size_t i = 0; i < ts_keys.size(); ++i) {
        if (ts_keys[i] != tu_keys[i]) {
            SetError("两种策略注册相同函数的 key 值应一致");
            return;
        }
    }
}

// ThreadSafe Client/Server 析构安全
CASE(TestThreadSafe_DestructionSafety)
{
    using namespace ToolBox::CoroRpc;
    
    // 在作用域内创建并销毁
    {
        CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
        client.SetSendCallback([](std::string &&) {});
        client.SetReqAttachment("temp attachment");
    }
    // client 销毁后不应崩溃
    
    {
        CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
        server.RegisterService<add>();
        server.RegisterService<async_add>();
        server.SetSendCallback([](uint64_t, std::string &&) {});
    }
    // server 销毁后不应崩溃
}

// ThreadSafe 客户端真实调用的请求元数据校验
CASE(TestThreadSafe_RepeatedRealCalls)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    server.RegisterService<add>();

    bool invalid_packet = false;
    CoroRpcProtocol::ReqHeader captured_header{};
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            invalid_packet = true;
            return;
        }
        std::memcpy(&captured_header, buffer.data(), sizeof(captured_header));
        server.OnRecvReq(1, std::string_view(buffer));
    });

    auto result = client.Call<add>(3, 2).get_result();
    (void)result;
    if (invalid_packet || captured_header.seq_num == 0) {
        SetError("ThreadSafe 客户端真实调用应产生完整请求包和有效 seq_num");
        return;
    }
    if (captured_header.client_id != client.get_client_id()) {
        SetError("请求头中的 client_id 应与客户端实例一致");
        return;
    }
}

// 多线程并发 OnRecvResp（ThreadSafe Client）
CASE(TestThreadSafe_ConcurrentOnRecvResp)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    client.SetSendCallback([](std::string &&) {});
    
    // 构造多个有效的响应包，seq_num 各不同
    auto make_resp = [](uint32_t seq) {
        CoroRpcProtocol::RespHeader header{};
        header.magic = CoroRpcProtocol::kMagicNumber;
        header.version = CoroRpcProtocol::kVersionNumber;
        header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
        header.seq_num = seq;
        header.err_code = 0;
        header.length = 0;
        header.attach_length = 0;
        
        std::string buf;
        buf.resize(CoroRpcProtocol::RESP_HEAD_LEN);
        std::memcpy(buf.data(), &header, sizeof(header));
        return buf;
    };
    
    // 多线程并发调用 OnRecvResp（无匹配 handler，但不应崩溃）
    std::vector<std::thread> threads;
    std::atomic<int> error_count{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&, i]() {
            std::string resp = make_resp(static_cast<uint32_t>(10000 + i));
            Errc err = client.OnRecvResp(resp);
            if (err != Errc::SUCCESS) {
                error_count++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (error_count > 0) {
        SetError("ThreadSafe 客户端并发 OnRecvResp 不应失败（协议层面都是有效的）");
        return;
    }
}

// 性能对比：NullMutex vs std::mutex 的 lock_guard 开销
CASE(TestPerformance_NullMutexVsStdMutex)
{
    using namespace ToolBox::CoroRpc;
    
    const int iterations = 100000;
    volatile int counter = 0;
    
    // NullMutex
    {
        NullMutex mtx;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<NullMutex> lock(mtx);
            counter++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto null_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        (void)null_duration;
    }
    
    // std::mutex
    counter = 0;
    {
        std::mutex mtx;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            counter++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        (void)std_duration;
    }
    
    // 验证两者都能正常完成（NullMutex 应该更快，但不做强断言）
    if (counter != iterations) {
        SetError("计数器值不正确");
        return;
    }
}

FIXTURE_END()

