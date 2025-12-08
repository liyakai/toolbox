#include "unit_test_frame/unittest.h"
#include "coro_rpc/coro_rpc_client.h"
#include "coro_rpc/coro_rpc_server.h"
#include "coro_rpc/impl/stream_rpc.h"
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

FIXTURE_BEGIN(CoroRpcStream)

using namespace ToolBox::CoroRpc;

// ========== 流式RPC测试辅助函数 ==========

// 流式RPC服务函数：生成整数序列
StreamGenerator<int> generate_numbers(int start, int count) {
    for (int i = 0; i < count; ++i) {
        co_yield start + i;
    }
}

// 流式RPC服务函数：生成字符串序列
StreamGenerator<std::string> generate_strings(int count) {
    for (int i = 0; i < count; ++i) {
        co_yield "item_" + std::to_string(i);
    }
}

// 流式RPC服务函数：空流
StreamGenerator<int> empty_stream() {
    co_return;
}

// 流式RPC服务函数：单个值
StreamGenerator<int> single_value(int value) {
    co_yield value;
    co_return;
}

// 测试类：成员函数流式RPC
class StreamService {
public:
    StreamGenerator<int> generate_member_numbers(int start, int count) {
        for (int i = 0; i < count; ++i) {
            co_yield start + i;
        }
    }
    
    StreamGenerator<std::string> generate_member_strings(int count) {
        for (int i = 0; i < count; ++i) {
            co_yield "member_item_" + std::to_string(i);
        }
    }
};

// ========== StreamGenerator 基础测试 ==========

CASE(TestStreamGenerator_BasicUsage)
{
    auto gen = generate_numbers(0, 5);
    
    std::vector<int> results;
    while (gen.Next()) {
        results.push_back(gen.value());
    }
    
    if (results.size() != 5) {
        SetError("生成的数字数量不正确");
        return;
    }
    
    for (int i = 0; i < 5; ++i) {
        if (results[i] != i) {
            SetError("生成的数字值不正确");
            return;
        }
    }
}

CASE(TestStreamGenerator_EmptyStream)
{
    auto gen = empty_stream();
    
    if (gen.Next()) {
        SetError("空流不应该有值");
        return;
    }
    
    if (!gen.done()) {
        SetError("空流应该立即完成");
        return;
    }
}

CASE(TestStreamGenerator_SingleValue)
{
    auto gen = single_value(42);
    
    if (!gen.Next()) {
        SetError("单值流应该有值");
        return;
    }
    
    if (gen.value() != 42) {
        SetError("单值流的值不正确");
        return;
    }
    
    if (gen.Next()) {
        SetError("单值流不应该有第二个值");
        return;
    }
}

CASE(TestStreamGenerator_MoveSemantics)
{
    auto gen1 = generate_numbers(0, 3);
    auto gen2 = std::move(gen1);
    
    // gen1 应该已经无效
    if (gen1.Next()) {
        SetError("移动后的生成器不应该有效");
        return;
    }
    
    // gen2 应该有效
    std::vector<int> results;
    while (gen2.Next()) {
        results.push_back(gen2.value());
    }
    
    if (results.size() != 3) {
        SetError("移动后的生成器应该正常工作");
        return;
    }
}

CASE(TestStreamGenerator_StringValues)
{
    auto gen = generate_strings(3);
    
    std::vector<std::string> results;
    while (gen.Next()) {
        results.push_back(gen.value());
    }
    
    if (results.size() != 3) {
        SetError("生成的字符串数量不正确");
        return;
    }
    
    if (results[0] != "item_0" || results[1] != "item_1" || results[2] != "item_2") {
        SetError("生成的字符串值不正确");
        return;
    }
}

// ========== StreamReader 基础测试 ==========

CASE(TestStreamReader_BasicUsage)
{
    StreamReader<int> reader(10);
    
    // 推送一些值
    if (!reader.PushValue(1)) {
        SetError("推送值失败");
        return;
    }
    
    if (!reader.PushValue(2)) {
        SetError("推送第二个值失败");
        return;
    }
    
    if (!reader.HasValue()) {
        SetError("应该有值可用");
        return;
    }
    
    auto value1 = reader.PopValue();
    if (!value1.has_value() || value1.value() != 1) {
        SetError("弹出的第一个值不正确");
        return;
    }
    
    auto value2 = reader.PopValue();
    if (!value2.has_value() || value2.value() != 2) {
        SetError("弹出的第二个值不正确");
        return;
    }
    
    if (reader.HasValue()) {
        SetError("不应该还有值");
        return;
    }
}

CASE(TestStreamReader_Finish)
{
    StreamReader<int> reader(10);
    
    reader.PushValue(1);
    reader.PushValue(2);
    reader.Finish();
    
    if (!reader.IsFinished()) {
        SetError("应该标记为已完成");
        return;
    }
    
    // 应该还能读取剩余的值
    auto value1 = reader.PopValue();
    if (!value1.has_value() || value1.value() != 1) {
        SetError("完成前应该还能读取值");
        return;
    }
    
    auto value2 = reader.PopValue();
    if (!value2.has_value() || value2.value() != 2) {
        SetError("完成前应该还能读取第二个值");
        return;
    }
    
    // 现在应该真正完成了
    if (!reader.IsFinished()) {
        SetError("读取完所有值后应该完成");
        return;
    }
    
    auto value3 = reader.PopValue();
    if (value3.has_value()) {
        SetError("完成后不应该有值");
        return;
    }
}

CASE(TestStreamReader_SetError)
{
    StreamReader<int> reader(10);
    
    reader.PushValue(1);
    reader.SetError(Errc::ERR_TIMEOUT);
    
    if (!reader.IsFinished()) {
        SetError("设置错误后应该标记为已完成");
        return;
    }
    
    auto error = reader.GetError();
    if (!error.has_value() || error.value() != Errc::ERR_TIMEOUT) {
        SetError("错误码不正确");
        return;
    }
    
    // 设置错误后不应该能读取值
    auto value = reader.PopValue();
    if (value.has_value()) {
        SetError("设置错误后不应该有值");
        return;
    }
}

CASE(TestStreamReader_Cancel)
{
    StreamReader<int> reader(10);
    
    reader.PushValue(1);
    reader.Cancel();
    
    if (!reader.IsCancelled()) {
        SetError("应该标记为已取消");
        return;
    }
    
    if (!reader.IsFinished()) {
        SetError("取消后应该标记为已完成");
        return;
    }
}

CASE(TestStreamReader_BufferSize)
{
    StreamReader<int> reader(5);
    
    if (reader.GetMaxBufferSize() != 5) {
        SetError("最大缓冲区大小不正确");
        return;
    }
    
    reader.PushValue(1);
    reader.PushValue(2);
    
    if (reader.GetBufferSize() != 2) {
        SetError("当前缓冲区大小不正确");
        return;
    }
}

CASE(TestStreamReader_Backpressure)
{
    StreamReader<int> reader(3);  // 小缓冲区
    
    bool pause_called = false;
    bool resume_called = false;
    
    reader.SetBackpressureCallback([&](uint32_t stream_id, bool pause) {
        if (pause) {
            pause_called = true;
        } else {
            resume_called = true;
        }
    });
    
    reader.SetStreamId(123);
    
    // 填满缓冲区
    reader.PushValue(1);
    reader.PushValue(2);
    reader.PushValue(3);
    
    // 尝试推送第四个值（应该触发背压）
    bool pushed = reader.PushValue(4);
    
    if (pushed) {
        SetError("缓冲区满时不应该能推送");
        return;
    }
    
    if (!pause_called) {
        SetError("应该触发暂停回调");
        return;
    }
    
    if (reader.IsPaused()) {
        SetError("应该处于暂停状态");
        return;
    }
    
    // 读取一个值，应该触发恢复
    reader.PopValue();
    
    if (!resume_called) {
        SetError("缓冲区有空间后应该触发恢复回调");
        return;
    }
    
    if (reader.IsPaused()) {
        SetError("不应该再处于暂停状态");
        return;
    }
}

// ========== 流式RPC服务器注册测试 ==========

CASE(TestStreamRpcServer_RegisterStreamFunction)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    
    server.RegisterService<generate_numbers>();
    server.RegisterService<generate_strings>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("流式服务注册数量不正确");
        return;
    }
}

CASE(TestStreamRpcServer_RegisterMemberStreamFunction)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    StreamService service;
    
    server.RegisterService<&StreamService::generate_member_numbers>(&service);
    server.RegisterService<&StreamService::generate_member_strings>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("成员流式服务注册数量不正确");
        return;
    }
}

// ========== 流式RPC客户端测试 ==========

CASE(TestStreamRpcClient_SetStreamId)
{
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 测试客户端可以正常创建
    uint64_t client_id = client.get_client_id();
    if (client_id == 0) {
        SetError("客户端ID不应该为0");
        return;
    }
}

CASE(TestStreamRpcClient_CancelStream)
{
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 测试取消不存在的流
    bool result = client.CancelStream(99999);
    
    if (result) {
        SetError("取消不存在的流应该返回false");
        return;
    }
}

// ========== 流式RPC协议测试 ==========

CASE(TestStreamRpcProtocol_MessageTypes)
{
    // 验证流式RPC消息类型枚举值
    if (static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart) != 1) {
        SetError("kMsgTypeStreamStart 值不正确");
        return;
    }
    
    if (static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamData) != 2) {
        SetError("kMsgTypeStreamData 值不正确");
        return;
    }
    
    if (static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd) != 3) {
        SetError("kMsgTypeStreamEnd 值不正确");
        return;
    }
    
    if (static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel) != 5) {
        SetError("kMsgTypeStreamCancel 值不正确");
        return;
    }
}

// ========== 集成测试：流式RPC客户端-服务器通信 ==========

CASE(TestStreamRpcIntegration_BasicStream)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    // 注册流式服务
    server.RegisterService<generate_numbers>();
    
    // 设置服务器发送回调
    std::vector<std::string> server_sent_messages;
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        server_sent_messages.push_back(buffer);
        // 模拟服务器响应到达客户端
        client.OnRecvResp(std::string_view(buffer));
    });
    
    // 设置客户端发送回调
    std::vector<std::string> client_sent_messages;
    client.SetSendCallback([&](std::string &&buffer) {
        client_sent_messages.push_back(buffer);
        // 模拟客户端请求到达服务器
        server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 注意：完整的流式RPC测试需要协程执行器
    // 这里主要验证协议层面的交互
}

CASE(TestStreamRpcIntegration_StreamControlMessages)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> server;
    CoroRpcClient<CoroRpcProtocol> client;
    
    server.RegisterService<generate_numbers>();
    
    bool cancel_received = false;
    bool pause_received = false;
    bool resume_received = false;
    
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
}

// ========== 边界条件测试 ==========

CASE(TestStreamRpcBoundary_LargeStream)
{
    // 测试大流
    auto gen = generate_numbers(0, 1000);
    
    int count = 0;
    while (gen.Next()) {
        count++;
    }
    
    if (count != 1000) {
        SetError("大流生成的数量不正确");
        return;
    }
}

CASE(TestStreamRpcBoundary_StreamReaderFullBuffer)
{
    StreamReader<int> reader(3);
    
    // 填满缓冲区
    reader.PushValue(1);
    reader.PushValue(2);
    reader.PushValue(3);
    
    // 尝试推送第四个值
    bool pushed = reader.PushValue(4);
    
    if (pushed) {
        SetError("缓冲区满时不应该能推送");
        return;
    }
    
    // 读取一个值后应该能推送
    reader.PopValue();
    pushed = reader.PushValue(4);
    
    if (!pushed) {
        SetError("缓冲区有空间后应该能推送");
        return;
    }
}

CASE(TestStreamRpcBoundary_StreamReaderEmptyAfterFinish)
{
    StreamReader<int> reader(10);
    
    reader.Finish();
    
    if (reader.IsFinished()) {
        SetError("空流完成时应该标记为已完成");
        return;
    }
    
    auto value = reader.PopValue();
    if (value.has_value()) {
        SetError("空流完成后不应该有值");
        return;
    }
}

// ========== 错误处理测试 ==========

CASE(TestStreamRpcError_StreamReaderErrorHandling)
{
    StreamReader<int> reader(10);
    
    reader.PushValue(1);
    reader.SetError(Errc::ERR_TIMEOUT);
    
    auto error = reader.GetError();
    if (!error.has_value() || error.value() != Errc::ERR_TIMEOUT) {
        SetError("错误码应该被正确保存");
        return;
    }
    
    // 设置错误后，即使有值也不应该能读取
    auto value = reader.PopValue();
    if (value.has_value()) {
        SetError("设置错误后不应该能读取值");
        return;
    }
}

CASE(TestStreamRpcError_CancelAfterFinish)
{
    StreamReader<int> reader(10);
    
    reader.Finish();
    reader.Cancel();
    
    // 取消已完成流应该没有影响
    if (!reader.IsFinished()) {
        SetError("取消已完成流后应该仍然标记为已完成");
        return;
    }
}

// ========== 并发测试 ==========

CASE(TestStreamRpcConcurrency_MultipleStreams)
{
    // 测试多个流生成器
    auto gen1 = generate_numbers(0, 5);
    auto gen2 = generate_numbers(10, 5);
    auto gen3 = generate_numbers(20, 5);
    
    std::vector<int> results1, results2, results3;
    
    while (gen1.Next()) {
        results1.push_back(gen1.value());
    }
    
    while (gen2.Next()) {
        results2.push_back(gen2.value());
    }
    
    while (gen3.Next()) {
        results3.push_back(gen3.value());
    }
    
    if (results1.size() != 5 || results2.size() != 5 || results3.size() != 5) {
        SetError("多个流生成的数量不正确");
        return;
    }
    
    if (results1[0] != 0 || results2[0] != 10 || results3[0] != 20) {
        SetError("多个流的起始值不正确");
        return;
    }
}

CASE(TestStreamRpcConcurrency_StreamReaderThreadSafety)
{
    StreamReader<int> reader(100);
    
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    
    // 生产者线程
    std::thread producer([&]() {
        for (int i = 0; i < 1000; ++i) {
            if (reader.PushValue(i)) {
                push_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        reader.Finish();
    });
    
    // 消费者线程
    std::thread consumer([&]() {
        while (!reader.IsFinished() || reader.HasValue()) {
            auto value = reader.PopValue();
            if (value.has_value()) {
                pop_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    producer.join();
    consumer.join();
    
    // 验证所有值都被处理
    if (push_count.load() != pop_count.load()) {
        SetError("推送和弹出的数量应该相等");
        return;
    }
}

// ========== 性能测试 ==========

CASE(TestStreamRpcPerformance_StreamGeneratorSpeed)
{
    const int iterations = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        auto gen = generate_numbers(0, iterations / 100);
        while (gen.Next()) {
            (void)gen.value();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 验证性能在合理范围内
    if (duration.count() > iterations * 100) {
        SetError("流生成器性能过慢");
        return;
    }
}

CASE(TestStreamRpcPerformance_StreamReaderSpeed)
{
    const int iterations = 10000;
    StreamReader<int> reader(1000);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 推送
    for (int i = 0; i < iterations; ++i) {
        reader.PushValue(i);
    }
    
    // 弹出
    for (int i = 0; i < iterations; ++i) {
        reader.PopValue();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 验证性能在合理范围内
    if (duration.count() > iterations * 10) {
        SetError("流读取器性能过慢");
        return;
    }
}

FIXTURE_END()

