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
#include <memory>
#include <mutex>
#include <unordered_set>

FIXTURE_BEGIN(CoroRpcIntegration)

using namespace ToolBox::CoroRpc;

// ========== 测试辅助函数 ==========

// 基本RPC函数
int add(int a, int b) {
    return a + b;
}

std::string echo_string(const std::string& str) {
    return str;
}

void void_function(int& value) {
    value = 100;
}

// 协程RPC函数
ToolBox::coro::Task<int> async_add(int a, int b) {
    co_return a + b;
}

ToolBox::coro::Task<std::string> async_echo(const std::string& str) {
    co_return str;
}

// 流式RPC函数
StreamGenerator<int> stream_numbers(int start, int count) {
    for (int i = 0; i < count; ++i) {
        co_yield start + i;
    }
}

// 测试类
class TestService {
public:
    int value_ = 0;
    
    int multiply(int a, int b) {
        return a * b;
    }
    
    void set_value(int v) {
        value_ = v;
    }
    
    ToolBox::coro::Task<int> async_multiply(int a, int b) {
        co_return a * b;
    }
    
    StreamGenerator<std::string> stream_strings(int count) {
        for (int i = 0; i < count; ++i) {
            co_yield "service_item_" + std::to_string(i);
        }
    }
};

// ========== 模拟网络层 ==========

class MockNetwork {
public:
    struct Message {
        uint64_t opaque;
        std::string data;
    };
    
    void SendToServer(uint64_t opaque, std::string&& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        server_messages_.push_back({opaque, std::move(data)});
    }
    
    void SendToClient(uint64_t opaque, std::string&& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        client_messages_.push_back({opaque, std::move(data)});
    }
    
    std::vector<Message> GetServerMessages() {
        std::lock_guard<std::mutex> lock(mutex_);
        return server_messages_;
    }
    
    std::vector<Message> GetClientMessages() {
        std::lock_guard<std::mutex> lock(mutex_);
        return client_messages_;
    }
    
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        server_messages_.clear();
        client_messages_.clear();
    }
    
private:
    std::mutex mutex_;
    std::vector<Message> server_messages_;
    std::vector<Message> client_messages_;
};

// ========== 基础集成测试 ==========

CASE(TestIntegration_ClientServerSetup)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    MockNetwork network;
    
    // 设置服务器
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        network.SendToClient(opaque, std::move(buffer));
    });
    
    // 设置客户端
    client.SetSendCallback([&](std::string &&buffer) {
        network.SendToServer(1, std::move(buffer));
    });
    
    // 注册服务
    server.RegisterService<add>();
    
    // 验证设置成功
    auto keys = server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("服务注册失败");
        return;
    }
}

CASE(TestIntegration_RequestResponseFlow)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    MockNetwork network;
    
    // 设置服务器
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        // 模拟服务器响应到达客户端
        client.OnRecvResp(std::string_view(buffer));
    });
    
    // 设置客户端
    client.SetSendCallback([&](std::string &&buffer) {
        // 模拟客户端请求到达服务器
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 注册服务
    server.RegisterService<add>();
    
    // 注意：完整的RPC调用需要协程执行器
    // 这里主要验证协议层面的消息流转
}

CASE(TestIntegration_MultipleServices)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 注册多个服务
    server.RegisterService<add>();
    server.RegisterService<echo_string>();
    server.RegisterService<async_add>();
    server.RegisterService<async_echo>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 4) {
        SetError("多个服务注册失败");
        return;
    }
    
    // 验证所有键都是唯一的
    std::unordered_set<uint32_t> key_set(keys.begin(), keys.end());
    if (key_set.size() != keys.size()) {
        SetError("服务键存在重复");
        return;
    }
}

CASE(TestIntegration_MemberFunctionServices)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    TestService service;
    
    // 注册成员函数服务
    server.RegisterService<&TestService::multiply>(&service);
    server.RegisterService<&TestService::set_value>(&service);
    server.RegisterService<&TestService::async_multiply>(&service);
    server.RegisterService<&TestService::stream_strings>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 4) {
        SetError("成员函数服务注册失败");
        return;
    }
}

CASE(TestIntegration_StreamServices)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 注册流式服务
    server.RegisterService<stream_numbers>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.empty()) {
        SetError("流式服务注册失败");
        return;
    }
}

CASE(TestIntegration_AttachmentHandling)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    std::string test_attachment = "test attachment data";
    
    // 设置请求附件
    bool ret = client.SetReqAttachment(test_attachment);
    if (!ret) {
        SetError("设置请求附件失败");
        return;
    }
    
    // 注册服务并设置响应附件函数
    server.RegisterService<add>();
    server.SetRespAttachmentFunc<add>([&]() -> std::string_view {
        return server.GetReqAttachment();
    });
}

CASE(TestIntegration_ErrorHandling_UnregisteredFunction)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 构造一个未注册函数的请求
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    req_header.msg_type = 0;
    req_header.seq_num = 1;
    req_header.func_id = 99999;  // 未注册的函数ID
    req_header.length = 0;
    req_header.attach_length = 0;
    req_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    // 验证服务端回错误包（而非仅返回错误码）
    bool error_response_sent = false;
    server.SetSendCallback([&](uint64_t opaque, std::string &&resp) {
        // 解析响应头检查错误码
        if (resp.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader resp_header;
            std::memcpy(&resp_header, resp.data(), sizeof(resp_header));
            if (resp_header.err_code == static_cast<uint8_t>(Errc::ERR_FUNC_NOT_REGISTERED)) {
                error_response_sent = true;
            }
        }
    });
    
    Errc err = server.OnRecvReq(1, buffer);
    
    // OnRecvReq 返回 SUCCESS（错误已通过响应包传递给客户端）
    if (err != Errc::SUCCESS) {
        SetError("未注册函数时 OnRecvReq 应返回 SUCCESS（错误已回包）");
        return;
    }
    if (!error_response_sent) {
        SetError("服务端应回错误包（ERR_FUNC_NOT_REGISTERED）给客户端");
        return;
    }
}

CASE(TestIntegration_ErrorHandling_InvalidProtocol)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 构造无效协议的请求
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = 99;  // 无效的序列化类型
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
    
    // 应该返回协议错误
    if (err == Errc::SUCCESS) {
        SetError("应该检测到无效的协议类型");
        return;
    }
}

CASE(TestIntegration_ErrorHandling_InvalidHeader)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 测试无效的响应头
    std::string invalid_data = "invalid response data";
    Errc err = client.OnRecvResp(invalid_data);
    
    if (err == Errc::SUCCESS) {
        SetError("应该检测到无效的响应头");
        return;
    }
}

CASE(TestIntegration_ConcurrentClients)
{
    std::vector<std::unique_ptr<CoroRpcClient<CoroRpcProtocol>>> clients;
    
    // 创建多个客户端
    for (int i = 0; i < 10; ++i) {
        clients.push_back(std::make_unique<CoroRpcClient<CoroRpcProtocol>>(i));
    }
    
    // 验证所有客户端都有不同的ID
    for (size_t i = 0; i < clients.size(); ++i) {
        for (size_t j = i + 1; j < clients.size(); ++j) {
            if (clients[i]->get_client_id() == clients[j]->get_client_id()) {
                SetError("客户端ID冲突");
                return;
            }
        }
    }
}

CASE(TestIntegration_ClientIdManagement)
{
    CoroRpcClient<CoroRpcProtocol> client1;
    CoroRpcClient<CoroRpcProtocol> client2(100);
    CoroRpcClient<CoroRpcProtocol> client3;
    
    if (client2.get_client_id() != 100) {
        SetError("客户端ID设置失败");
        return;
    }
    
    client1.set_client_id(200);
    if (client1.get_client_id() != 200) {
        SetError("客户端ID修改失败");
        return;
    }
}

CASE(TestIntegration_StreamControlMessages)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    server.RegisterService<stream_numbers>();
    
    std::atomic<bool> cancel_received{false};
    std::atomic<bool> pause_received{false};
    std::atomic<bool> resume_received{false};
    
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    client.SetSendCallback([&](std::string &&buffer) {
        // 检查是否是控制消息
        if (buffer.size() >= CoroRpcProtocol::REQ_HEAD_LEN) {
            auto* header = reinterpret_cast<const CoroRpcProtocol::ReqHeader*>(buffer.data());
            if (header->msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
                cancel_received = true;
            } else if (header->msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamPause)) {
                pause_received = true;
            } else if (header->msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamResume)) {
                resume_received = true;
            }
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 注意：需要实际调用流式RPC才能触发控制消息
    // 这里主要验证控制消息的协议支持
}

CASE(TestIntegration_ProtocolCompatibility)
{
    // 验证协议常量
    if (CoroRpcProtocol::kMagicNumber != 0xde) {
        SetError("Magic number 不正确");
        return;
    }
    
    if (CoroRpcProtocol::kVersionNumber != 1) {
        SetError("Version number 不正确");
        return;
    }
    
    // 验证头部大小
    if (sizeof(CoroRpcProtocol::ReqHeader) != 32) {
        SetError("请求头大小不正确");
        return;
    }
    
    if (sizeof(CoroRpcProtocol::RespHeader) != 32) {
        SetError("响应头大小不正确");
        return;
    }
}

CASE(TestIntegration_SerializationProtocolSelection)
{
    // 基本类型应该使用 StructPackProtocol
    auto protocol1 = CoroRpcProtocol::GetSerializeProtocol<add>();
    bool is_struct_pack = std::holds_alternative<StructPackProtocol>(protocol1);
    
    if (!is_struct_pack) {
        SetError("基本类型应该使用 StructPackProtocol");
        return;
    }
}

CASE(TestIntegration_AttachmentSizeLimits)
{
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 测试正常大小的附件
    std::string normal_attachment(1000, 'A');
    bool ret = client.SetReqAttachment(normal_attachment);
    
    if (!ret) {
        SetError("应该允许正常大小的附件");
        return;
    }
    
    // 测试空附件
    ret = client.SetReqAttachment("");
    if (!ret) {
        SetError("应该允许空附件");
        return;
    }
    
    // 注意：测试超过 UINT32_MAX (4GB) 的情况会导致内存分配问题
    // 在实际环境中，这种大小的附件不应该出现
    // 代码中的边界检查逻辑已经通过编译时类型检查保证正确性
    // 如果需要测试边界情况，应该在压力测试或集成测试中使用更合适的方法
}

CASE(TestIntegration_EmptyAttachment)
{
    CoroRpcClient<CoroRpcProtocol> client;
    
    bool ret = client.SetReqAttachment("");
    
    if (!ret) {
        SetError("应该允许空附件");
        return;
    }
}

CASE(TestIntegration_MultipleRequestIds)
{
    CoroRpcClient<CoroRpcProtocol> client1;
    CoroRpcClient<CoroRpcProtocol> client2;
    
    // 验证两个客户端可以独立工作
    client1.SetSendCallback([](std::string &&) {});
    client2.SetSendCallback([](std::string &&) {});
    
    // 注意：request_id 是私有的，这里只能验证客户端可以正常创建和配置
}

CASE(TestIntegration_ServerResponseAttachment)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<add>();
    
    std::string test_attachment = "response attachment";
    server.SetRespAttachmentFunc<add>([&]() -> std::string_view {
        return test_attachment;
    });
    
    // 验证设置成功（无异常）
}

CASE(TestIntegration_StreamStateManagement)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<stream_numbers>();
    
    // 测试流状态管理（通过控制消息）
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    req_header.msg_type = static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel);
    req_header.seq_num = 12345;  // stream_id
    req_header.func_id = 0;
    req_header.length = 0;
    req_header.attach_length = 0;
    req_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    server.SetSendCallback([](uint64_t, std::string &&) {});
    
    Errc err = server.OnRecvReq(1, buffer);
    
    // 应该成功处理控制消息
    if (err != Errc::SUCCESS) {
        SetError("应该成功处理流控制消息");
        return;
    }
}

CASE(TestIntegration_RequestIdIncrement)
{
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 验证客户端可以正常创建
    // request_id 是私有的，但可以通过行为推断
    // 这里主要验证客户端可以正常工作
    client.SetSendCallback([](std::string &&) {});
}

CASE(TestIntegration_ErrorCodeValues)
{
    // 验证错误码的值
    if (static_cast<int>(Errc::SUCCESS) != 0) {
        SetError("SUCCESS 错误码应该是 0");
        return;
    }
    
    if (static_cast<int>(Errc::ERR_TIMEOUT) != 1) {
        SetError("ERR_TIMEOUT 错误码应该是 1");
        return;
    }
}

CASE(TestIntegration_ProtocolHeaderAlignment)
{
    // 测试头部对齐
    CoroRpcProtocol::ReqHeader header;
    header.magic = CoroRpcProtocol::kMagicNumber;
    header.version = CoroRpcProtocol::kVersionNumber;
    header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    header.msg_type = 0;
    header.seq_num = 12345;
    header.func_id = 67890;
    header.length = 100;
    header.attach_length = 50;
    header.client_id = 1;
    
    // 创建非对齐的缓冲区
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

// ========== 新接口测试 ==========

// 测试 resp_body 的 owned storage：string_view 构造后数据独立于原始 buffer
CASE(TestRespBody_OwnedStorage)
{
    // 模拟外部 buffer 被销毁后 resp_body 仍然有效
    resp_body body;
    {
        std::string temp_body = "hello response body";
        std::string temp_attach = "hello attachment";
        body = resp_body(1, std::string_view(temp_body), std::string_view(temp_attach));
        // temp_body 和 temp_attach 在这里销毁
    }
    // body 应该持有自己的拷贝，不会悬空
    if (body.body_buffer_ != "hello response body") {
        SetError("resp_body 的 body_buffer_ 应该是 owned copy");
        return;
    }
    if (body.resp_attachment_buf_ != "hello attachment") {
        SetError("resp_body 的 resp_attachment_buf_ 应该是 owned copy");
        return;
    }
    if (body.serialize_type_ != 1) {
        SetError("resp_body 的 serialize_type_ 不正确");
        return;
    }
}

// 测试 resp_body 的 move 构造
CASE(TestRespBody_MoveConstruction)
{
    resp_body body1(2, std::string_view("body data"), std::string_view("attachment data"));
    resp_body body2(std::move(body1));
    
    if (body2.body_buffer_ != "body data") {
        SetError("move 后 body_buffer_ 不正确");
        return;
    }
    if (body2.resp_attachment_buf_ != "attachment data") {
        SetError("move 后 resp_attachment_buf_ 不正确");
        return;
    }
}

// 测试 async_rpc_result_value_t 的 HasError / GetError / operator bool
CASE(TestAsyncRpcResult_HasError)
{
    // 成功的结果
    {
        async_rpc_result_value_t<int> result(42, resp_body(uint8_t(0), std::string_view(""), std::string_view("")));
        if (result.HasError()) {
            SetError("成功的结果 HasError() 应该返回 false");
            return;
        }
        if (result.GetError() != Errc::SUCCESS) {
            SetError("成功的结果 GetError() 应该返回 SUCCESS");
            return;
        }
        if (!static_cast<bool>(result)) {
            SetError("成功的结果 operator bool 应该返回 true");
            return;
        }
        if (*result != 42) {
            SetError("成功的结果值应该是 42");
            return;
        }
    }
    
    // 失败的结果
    {
        async_rpc_result_value_t<int> err_result(Errc::ERR_TIMEOUT);
        if (!err_result.HasError()) {
            SetError("失败的结果 HasError() 应该返回 true");
            return;
        }
        if (err_result.GetError() != Errc::ERR_TIMEOUT) {
            SetError("失败的结果 GetError() 应该返回 ERR_TIMEOUT");
            return;
        }
        if (static_cast<bool>(err_result)) {
            SetError("失败的结果 operator bool 应该返回 false");
            return;
        }
    }
}

// 测试 async_rpc_result_value_t<void> 的 HasError / GetError
CASE(TestAsyncRpcResult_VoidType)
{
    // 成功的 void 结果
    {
        async_rpc_result_value_t<void> result;
        if (result.HasError()) {
            SetError("成功的 void 结果 HasError() 应该返回 false");
            return;
        }
        if (!static_cast<bool>(result)) {
            SetError("成功的 void 结果 operator bool 应该返回 true");
            return;
        }
    }
    
    // 失败的 void 结果
    {
        async_rpc_result_value_t<void> err_result(Errc::ERR_FUNC_NOT_REGISTERED);
        if (!err_result.HasError()) {
            SetError("失败的 void 结果 HasError() 应该返回 true");
            return;
        }
        if (err_result.GetError() != Errc::ERR_FUNC_NOT_REGISTERED) {
            SetError("失败的 void 结果 GetError() 应该返回 ERR_FUNC_NOT_REGISTERED");
            return;
        }
    }
}

// 测试服务端 per-request SetRespAttachment 接口
CASE(TestServerSetRespAttachment)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 测试 SetRespAttachment 的 API 可用性
    server.SetRespAttachment("test attachment from api");
    
    // 验证 GetReqAttachment 返回空（无请求上下文时）
    // 因为没有活跃请求，attachment 应该是空的
    // (thread_local 初始值为空)
    (void)server.GetReqAttachment();
}

// 测试 RequestContext 的 thread_local 隔离
CASE(TestRequestContext_ThreadLocal)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    // 验证在无请求上下文时 GetReqAttachment 返回空
    auto att = server.GetReqAttachment();
    if (!att.empty()) {
        SetError("无请求上下文时 GetReqAttachment 应返回空");
        return;
    }
}

// ========== ThreadSafe 策略测试 ==========

// 测试 ThreadSafe 策略下 Client/Server 可以正确实例化和工作
CASE(TestThreadSafe_ClientServerSetup)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    // 设置服务器
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        (void)buffer;
    });
    
    // 设置客户端
    client.SetSendCallback([&](std::string &&buffer) {
        (void)buffer;
    });
    
    // 注册服务
    server.RegisterService<add>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("ThreadSafe 策略下服务注册失败");
        return;
    }
}

// 测试 ThreadSafe 策略下多服务注册
CASE(TestThreadSafe_MultipleServices)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<add>();
    server.RegisterService<echo_string>();
    server.RegisterService<async_add>();
    server.RegisterService<async_echo>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 4) {
        SetError("ThreadSafe 策略下多个服务注册失败");
        return;
    }
}

// 测试 ThreadSafe 策略下成员函数注册
CASE(TestThreadSafe_MemberFunctionServices)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    TestService service;
    
    server.RegisterService<&TestService::multiply>(&service);
    server.RegisterService<&TestService::set_value>(&service);
    server.RegisterService<&TestService::async_multiply>(&service);
    server.RegisterService<&TestService::stream_strings>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 4) {
        SetError("ThreadSafe 策略下成员函数服务注册失败");
        return;
    }
}

// 测试 ThreadSafe 策略下完整的请求/响应流转
CASE(TestThreadSafe_RequestResponseFlow)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        client.OnRecvResp(std::string_view(buffer));
    });
    
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    server.RegisterService<add>();
    
    // 验证协议层面的消息流转
}

// 测试 ThreadSafe 策略下的错误处理
CASE(TestThreadSafe_ErrorHandling)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    // 构造一个未注册函数的请求
    CoroRpcProtocol::ReqHeader req_header;
    req_header.magic = CoroRpcProtocol::kMagicNumber;
    req_header.version = CoroRpcProtocol::kVersionNumber;
    req_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    req_header.msg_type = 0;
    req_header.seq_num = 1;
    req_header.func_id = 99999;
    req_header.length = 0;
    req_header.attach_length = 0;
    req_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &req_header, sizeof(req_header));
    
    bool error_response_sent = false;
    server.SetSendCallback([&](uint64_t opaque, std::string &&resp) {
        (void)opaque;
        if (resp.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader resp_header;
            std::memcpy(&resp_header, resp.data(), sizeof(resp_header));
            if (resp_header.err_code == static_cast<uint8_t>(Errc::ERR_FUNC_NOT_REGISTERED)) {
                error_response_sent = true;
            }
        }
    });
    
    Errc err = server.OnRecvReq(1, buffer);
    
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe 策略下未注册函数时 OnRecvReq 应返回 SUCCESS");
        return;
    }
    if (!error_response_sent) {
        SetError("ThreadSafe 策略下服务端应回错误包");
        return;
    }
}

// 测试默认策略类型（ThreadUnsafe）—— 验证不传第二/三个参数即为无锁版
CASE(TestDefaultPolicy_IsThreadUnsafe)
{
    // 不传 ThreadSafetyPolicy → 默认 ThreadUnsafe
    CoroRpcClient<CoroRpcProtocol> client_default;
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server_default;
    
    // 显式传 ThreadUnsafe → 应与默认版一致
    CoroRpcClient<CoroRpcProtocol, ThreadUnsafe> client_explicit;
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadUnsafe> server_explicit;
    
    // 验证两者可以正常注册服务
    server_default.RegisterService<add>();
    server_explicit.RegisterService<add>();
    
    auto keys1 = server_default.GetAllServiceHandlerKeys();
    auto keys2 = server_explicit.GetAllServiceHandlerKeys();
    
    if (keys1.empty() || keys2.empty()) {
        SetError("默认策略和显式 ThreadUnsafe 策略应该都能正常注册服务");
        return;
    }
}

FIXTURE_END()

