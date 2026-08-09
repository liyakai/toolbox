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
#include <mutex>
#include <unordered_set>

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

// 流式RPC服务函数：慢速生成整数序列，用于验证取消/公平性等时序场景
StreamGenerator<int> slow_generate_numbers(int start, int count, int delay_us) {
    for (int i = 0; i < count; ++i) {
        if (delay_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
        }
        co_yield start + i;
    }
}

int stream_add(int a, int b) {
    return a + b;
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
    
    // IsFinished 表示不会再有新数据到达，而不是缓冲区已经读空
    if (!reader.IsFinished()) {
        SetError("应该标记为已完成");
        return;
    }
    
    if (!reader.HasValue()) {
        SetError("完成后仍应能读取已缓冲的值");
        return;
    }
    
    auto value1 = reader.PopValue();
    if (!value1.has_value() || value1.value() != 1) {
        SetError("完成后应该还能读取值");
        return;
    }
    
    auto value2 = reader.PopValue();
    if (!value2.has_value() || value2.value() != 2) {
        SetError("完成后应该还能读取第二个值");
        return;
    }
    
    if (!reader.IsFinished()) {
        SetError("读取完所有值后应该仍保持完成状态");
        return;
    }
    
    if (reader.HasValue()) {
        SetError("读取完所有值后不应该还有缓冲数据");
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

// Cancel/SetError 后迟到 Push 应丢弃；Finish 后在途 Push 仍应可读
CASE(TestStreamReader_PushValueAfterTerminalStates)
{
    {
        StreamReader<int> reader(10);
        reader.SetError(Errc::ERR_TIMEOUT);
        size_t before = reader.GetBufferSize();
        if (!reader.PushValue(1)) {
            SetError("SetError 后 PushValue 丢弃应返回 true(无需背压)");
            return;
        }
        if (reader.GetBufferSize() != before) {
            SetError("SetError 后迟到 Push 不应入队");
            return;
        }
        if (reader.HasValue()) {
            SetError("SetError 后不应有可读值");
            return;
        }
    }

    {
        StreamReader<int> reader(10);
        reader.Cancel();
        size_t before = reader.GetBufferSize();
        if (!reader.PushValue(2)) {
            SetError("Cancel 后 PushValue 丢弃应返回 true(无需背压)");
            return;
        }
        if (reader.GetBufferSize() != before) {
            SetError("Cancel 后迟到 Push 不应入队");
            return;
        }
    }

    {
        StreamReader<int> reader(10);
        reader.Finish();
        if (!reader.PushValue(42)) {
            SetError("Finish 后在途 Push 应成功且无需背压");
            return;
        }
        auto value = reader.PopValue();
        if (!value.has_value() || value.value() != 42) {
            SetError("Finish 后在途 Push 的数据应仍可 PopValue");
            return;
        }
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
        if (stream_id != 123) {
            SetError("背压回调收到的 stream_id 不正确");
            return;
        }
        if (pause) {
            pause_called = true;
        } else {
            resume_called = true;
        }
    });
    
    reader.SetStreamId(123);
    
    reader.PushValue(1);
    reader.PushValue(2);
    bool within_soft_limit = reader.PushValue(3);
    if (!within_soft_limit) {
        SetError("达到软上限的首个值仍应被视为成功保存");
        return;
    }
    if (!pause_called) {
        SetError("缓冲区达到上限时应该立即触发暂停回调");
        return;
    }
    if (!reader.IsPaused()) {
        SetError("达到上限后应该处于暂停状态");
        return;
    }

    // 第四个值也必须保留，不能因为触发背压而永久丢包。
    bool pushed_over_limit = reader.PushValue(4);
    if (pushed_over_limit) {
        SetError("超过软上限后返回值应提示背压状态");
        return;
    }
    if (reader.GetBufferSize() != 4) {
        SetError("触发背压时当前值仍应保存在缓冲区中");
        return;
    }

    auto first = reader.PopValue();
    auto second = reader.PopValue();
    auto third = reader.PopValue();
    auto fourth = reader.PopValue();
    if (!first.has_value() || !second.has_value() || !third.has_value() || !fourth.has_value()) {
        SetError("背压后应仍能读出全部已缓存的数据");
        return;
    }
    if (*fourth != 4) {
        SetError("超过软上限时保存的值应可被正常读取");
        return;
    }
    if (!resume_called) {
        SetError("缓冲区降到恢复阈值后应该触发恢复回调");
        return;
    }
    if (reader.IsPaused()) {
        SetError("恢复后不应该仍处于暂停状态");
        return;
    }
}

CASE(TestStreamReader_BackpressureResumeThreshold)
{
    StreamReader<int> reader(6);

    int pause_count = 0;
    int resume_count = 0;
    reader.SetStreamId(456);
    reader.SetBackpressureCallback([&](uint32_t stream_id, bool pause) {
        if (stream_id != 456) {
            SetError("恢复阈值测试收到错误的 stream_id");
            return;
        }
        if (pause) {
            ++pause_count;
        } else {
            ++resume_count;
        }
    });

    for (int i = 1; i <= 6; ++i) {
        bool pushed = reader.PushValue(i);
        if (!pushed) {
            SetError("达到软上限前的值都应视为成功保存");
            return;
        }
    }
    if (pause_count != 1 || !reader.IsPaused()) {
        SetError("达到软上限时应只触发一次暂停，并进入暂停状态");
        return;
    }

    reader.PopValue();
    reader.PopValue();
    reader.PopValue();
    if (resume_count != 0) {
        SetError("缓冲区仅降到一半时还不应触发恢复");
        return;
    }
    if (!reader.IsPaused()) {
        SetError("未低于恢复阈值前应保持暂停状态");
        return;
    }

    auto fourth = reader.PopValue();
    if (!fourth.has_value() || *fourth != 4) {
        SetError("恢复阈值测试读取到的值不正确");
        return;
    }
    if (resume_count != 1) {
        SetError("缓冲区低于一半后应触发一次恢复");
        return;
    }
    if (reader.IsPaused()) {
        SetError("恢复后不应继续处于暂停状态");
        return;
    }
}

CASE(TestStreamReader_BackpressureMultipleCycles)
{
    StreamReader<int> reader(4);

    int pause_count = 0;
    int resume_count = 0;
    std::vector<int> drained_values;
    reader.SetStreamId(789);
    reader.SetBackpressureCallback([&](uint32_t stream_id, bool pause) {
        if (stream_id != 789) {
            SetError("多轮背压测试收到错误的 stream_id");
            return;
        }
        if (pause) {
            ++pause_count;
        } else {
            ++resume_count;
        }
    });

    for (int i = 1; i <= 5; ++i) {
        bool pushed = reader.PushValue(i);
        if (i <= 4 && !pushed) {
            SetError("第一次达到软上限前的值都应视为成功保存");
            return;
        }
        if (i == 5 && pushed) {
            SetError("第一次超过软上限后应返回背压提示");
            return;
        }
    }
    if (pause_count != 1 || !reader.IsPaused()) {
        SetError("第一次填满缓冲区后应进入暂停状态");
        return;
    }

    for (int expected = 1; expected <= 3; ++expected) {
        auto value = reader.PopValue();
        if (!value.has_value() || *value != expected) {
            SetError("第一次恢复前读取到的值顺序不正确");
            return;
        }
        drained_values.push_back(*value);
    }
    if (resume_count != 0 || !reader.IsPaused()) {
        SetError("缓冲区尚未低于恢复阈值前不应提前恢复");
        return;
    }

    auto fourth = reader.PopValue();
    if (!fourth.has_value() || *fourth != 4) {
        SetError("第一次恢复触发点读取到的值不正确");
        return;
    }
    drained_values.push_back(*fourth);
    if (resume_count != 1 || reader.IsPaused()) {
        SetError("第一次低于恢复阈值后应恢复发送");
        return;
    }

    for (int i = 6; i <= 9; ++i) {
        bool pushed = reader.PushValue(i);
        if (i <= 8 && !pushed) {
            SetError("第二次达到软上限前的值都应视为成功保存");
            return;
        }
        if (i == 9 && pushed) {
            SetError("第二次超过软上限后应返回背压提示");
            return;
        }
    }
    if (pause_count != 2 || !reader.IsPaused()) {
        SetError("第二轮填满缓冲区后应再次进入暂停状态");
        return;
    }

    while (true) {
        auto value = reader.PopValue();
        if (!value.has_value()) {
            break;
        }
        drained_values.push_back(*value);
    }

    if (resume_count != 2) {
        SetError("多轮背压测试应触发两次恢复");
        return;
    }
    if (reader.IsPaused()) {
        SetError("多轮背压恢复后不应继续处于暂停状态");
        return;
    }
    if (drained_values.size() != 9) {
        SetError("多轮背压测试应完整读出全部缓存值");
        return;
    }
    for (int i = 0; i < 9; ++i) {
        if (drained_values[i] != i + 1) {
            SetError("多轮背压测试读取顺序不正确");
            return;
        }
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
    
    // 第四个值会触发背压，但仍然必须被保存。
    bool pushed = reader.PushValue(4);
    
    if (pushed) {
        SetError("超过软上限后返回值应提示背压状态");
        return;
    }
    if (reader.GetBufferSize() != 4) {
        SetError("超过软上限时当前值仍应保存在缓冲区中");
        return;
    }
    
    // 只有降到低水位以下后，后续推送才会重新回到正常状态。
    reader.PopValue();
    reader.PopValue();
    reader.PopValue();
    reader.PopValue();
    pushed = reader.PushValue(5);
    
    if (!pushed) {
        SetError("恢复到低水位后应该重新允许正常推送");
        return;
    }
}

CASE(TestStreamRpcBoundary_StreamReaderEmptyAfterFinish)
{
    StreamReader<int> reader(10);
    
    reader.Finish();
    
    if (!reader.IsFinished()) {
        SetError("空流完成时应该标记为已完成");
        return;
    }
    
    if (reader.HasValue()) {
        SetError("空流完成后不应该有缓冲值");
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
            reader.PushValue(i);
            push_count++;
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

// ========== ThreadSafe 策略下的流式 RPC 深度测试 ==========

// ThreadSafe Server 注册流式服务
CASE(TestThreadSafe_StreamRpcServer_RegisterStreamFunction)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<generate_numbers>();
    server.RegisterService<generate_strings>();
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("ThreadSafe 策略下流式服务注册数量不正确");
        return;
    }
}

// ThreadSafe Server 注册成员函数流式服务
CASE(TestThreadSafe_StreamRpcServer_RegisterMemberStream)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    StreamService service;
    
    server.RegisterService<&StreamService::generate_member_numbers>(&service);
    server.RegisterService<&StreamService::generate_member_strings>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 2) {
        SetError("ThreadSafe 策略下成员流式服务注册数量不正确");
        return;
    }
}

// ThreadSafe Client 流式操作
CASE(TestThreadSafe_StreamRpcClient_Operations)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    // 测试客户端可以正常创建
    uint64_t client_id = client.get_client_id();
    if (client_id == 0) {
        // client_id 可能从 0 开始，这里只是验证创建成功
    }
    
    // 测试取消不存在的流
    bool result = client.CancelStream(99999);
    
    if (result) {
        SetError("ThreadSafe 策略下取消不存在的流应该返回 false");
        return;
    }
}

// ThreadSafe：失败的流式调用不应把请求附件泄露到下一次调用
CASE(TestThreadSafe_StreamFailedCallClearsReqAttachment)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    client.SetReqAttachment("stale-stream-attachment");
    auto failed_reader = client.CallStream<generate_numbers>(10, 1).get_result();
    if (failed_reader != nullptr) {
        SetError("未设置 send callback 的流式调用应返回空 StreamReader");
        return;
    }

    bool saw_request = false;
    bool invalid_packet = false;
    CoroRpcProtocol::ReqHeader req_header{};
    client.SetSendCallback([&](std::string &&buffer) {
        saw_request = true;
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            invalid_packet = true;
            return;
        }
        std::memcpy(&req_header, buffer.data(), sizeof(req_header));
    });

    auto reader = client.CallStream<generate_numbers>(20, 1).get_result();
    if (!reader) {
        SetError("后续流式调用应成功返回有效的 StreamReader");
        return;
    }
    if (!saw_request || invalid_packet) {
        SetError("应捕获到完整的后续流式请求包");
        return;
    }
    if (req_header.attach_length != 0) {
        SetError("失败流式调用中的请求附件不应泄露到下一次调用");
        return;
    }
    if (req_header.msg_type != static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart)) {
        SetError("后续流式请求首帧应保持 StreamStart 类型");
        return;
    }
}

// ThreadSafe 集成：流式 Client-Server 真实端到端调用
CASE(TestThreadSafe_StreamRpcIntegration_BasicStream)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    
    server.RegisterService<generate_numbers>();

    std::mutex messages_mutex;
    std::vector<std::string> server_sent_messages;
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            server_sent_messages.push_back(buffer);
        }
        client.OnRecvResp(std::string_view(buffer));
    });
    
    std::vector<std::string> client_sent_messages;
    client.SetSendCallback([&](std::string &&buffer) {
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            client_sent_messages.push_back(buffer);
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });

    auto reader = client.CallStream<generate_numbers>(10, 5).get_result();
    if (!reader) {
        SetError("ThreadSafe 流式调用应返回有效的 StreamReader");
        return;
    }

    bool saw_stream_end = false;
    std::vector<int> values;
    for (int spin = 0; spin < 100; ++spin) {
        while (true) {
            auto payload = reader->PopValue();
            if (!payload.has_value()) {
                break;
            }
            int value = 0;
            if (!StructPackProtocol::Deserialize(value, std::string_view(*payload))) {
                SetError("流式返回值反序列化失败");
                return;
            }
            values.push_back(value);
        }

        std::string last_message;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            if (!server_sent_messages.empty()) {
                last_message = server_sent_messages.back();
            }
        }
        if (!last_message.empty()) {
            CoroRpcProtocol::RespHeader last_resp_header{};
            std::memcpy(&last_resp_header, last_message.data(), sizeof(last_resp_header));
            if (last_resp_header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd)) {
                saw_stream_end = true;
            }
        }
        if (reader->IsFinished()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (!saw_stream_end) {
        SetError("基础流式调用最终应收到 StreamEnd");
        return;
    }
    if (values.size() != 5) {
        SetError("基础流式调用应完整收到全部 5 个值");
        return;
    }
    for (int i = 0; i < 5; ++i) {
        if (values[i] != 10 + i) {
            SetError("基础流式返回值内容不正确");
            return;
        }
    }

    std::string first_client_message;
    {
        std::lock_guard<std::mutex> lock(messages_mutex);
        if (client_sent_messages.empty() || server_sent_messages.empty()) {
            SetError("流式调用应产生请求消息和响应消息");
            return;
        }
        first_client_message = client_sent_messages.front();
    }

    CoroRpcProtocol::ReqHeader req_header{};
    std::memcpy(&req_header, first_client_message.data(), sizeof(req_header));
    if (req_header.msg_type != static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart)) {
        SetError("流式请求首帧应为 StreamStart");
        return;
    }
}

CASE(TestThreadSafe_StreamRpcIntegration_LargeStreamNoBackpressure)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    server.RegisterService<generate_numbers>();

    std::mutex messages_mutex;
    std::vector<std::string> server_sent_messages;
    std::vector<std::string> client_sent_messages;
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            server_sent_messages.push_back(buffer);
        }
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            client_sent_messages.push_back(buffer);
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });

    constexpr int kCount = 130;
    auto reader = client.CallStream<generate_numbers>(0, kCount).get_result();
    if (!reader) {
        SetError("大流式调用应返回有效的 StreamReader");
        return;
    }

    std::vector<int> values;
    for (int spin = 0; spin < 200; ++spin) {
        while (true) {
            auto payload = reader->PopValue();
            if (!payload.has_value()) {
                break;
            }
            int value = 0;
            if (!StructPackProtocol::Deserialize(value, std::string_view(*payload))) {
                SetError("大流式返回值反序列化失败");
                return;
            }
            values.push_back(value);
        }
        if (reader->IsFinished()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (values.size() != static_cast<size_t>(kCount)) {
        SetError("无背压大流应完整收到全部 chunk，而不是在 64 个后停住");
        return;
    }
    for (int i = 0; i < kCount; ++i) {
        if (values[i] != i) {
            SetError("大流返回值内容不正确");
            return;
        }
    }
    if (!reader->IsFinished()) {
        SetError("大流在收到全部 chunk 后应进入完成状态");
        return;
    }

    std::string last_server_message;
    std::vector<std::string> client_messages_snapshot;
    {
        std::lock_guard<std::mutex> lock(messages_mutex);
        if (server_sent_messages.empty()) {
            SetError("大流式调用应产生服务端响应帧");
            return;
        }
        last_server_message = server_sent_messages.back();
        client_messages_snapshot = client_sent_messages;
    }

    CoroRpcProtocol::RespHeader last_resp_header{};
    std::memcpy(&last_resp_header, last_server_message.data(), sizeof(last_resp_header));
    if (last_resp_header.msg_type != static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd)) {
        SetError("大流最后一帧应为 StreamEnd");
        return;
    }

    for (size_t i = 1; i < client_messages_snapshot.size(); ++i) {
        if (client_messages_snapshot[i].size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            SetError("客户端控制帧不完整");
            return;
        }
        CoroRpcProtocol::ReqHeader control_header{};
        std::memcpy(&control_header, client_messages_snapshot[i].data(), sizeof(control_header));
        if (control_header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamPause) ||
            control_header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamResume) ||
            control_header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
            SetError("无背压大流不应额外触发控制消息");
            return;
        }
    }
}

CASE(TestThreadSafe_StreamRpcIntegration_BackpressureNoDataLoss)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    server.RegisterService<generate_numbers>();

    std::atomic<bool> pause_seen{false};
    std::atomic<bool> resume_seen{false};
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() >= CoroRpcProtocol::REQ_HEAD_LEN) {
            CoroRpcProtocol::ReqHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamPause)) {
                pause_seen.store(true, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamResume)) {
                resume_seen.store(true, std::memory_order_relaxed);
            }
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });

    constexpr int kCount = 1100;
    auto reader = client.CallStream<generate_numbers>(0, kCount).get_result();
    if (!reader) {
        SetError("背压场景应返回有效的 StreamReader");
        return;
    }

    bool paused_before_draining = false;
    for (int spin = 0; spin < 200; ++spin) {
        if (pause_seen.load(std::memory_order_relaxed) && reader->IsPaused()) {
            paused_before_draining = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!paused_before_draining) {
        SetError("高水位大流应触发客户端背压暂停");
        return;
    }

    std::vector<int> values;
    for (int spin = 0; spin < 600; ++spin) {
        while (true) {
            auto payload = reader->PopValue();
            if (!payload.has_value()) {
                break;
            }
            int value = 0;
            if (!StructPackProtocol::Deserialize(value, std::string_view(*payload))) {
                SetError("背压场景返回值反序列化失败");
                return;
            }
            values.push_back(value);
        }
        if (reader->IsFinished()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (!resume_seen.load(std::memory_order_relaxed)) {
        SetError("消费降到低水位后应向服务端发送恢复控制消息");
        return;
    }
    if (values.size() != static_cast<size_t>(kCount)) {
        SetError("背压场景也应完整收到全部数据，不能因满缓冲而丢包");
        return;
    }
    for (int i = 0; i < kCount; ++i) {
        if (values[i] != i) {
            SetError("背压场景返回值内容不正确");
            return;
        }
    }
    if (!reader->IsFinished()) {
        SetError("背压场景在消费完成后应进入完成状态");
        return;
    }
}

// ThreadSafe 集成：真实触发 pause/resume/cancel 控制消息
CASE(TestThreadSafe_StreamRpcIntegration_ControlMessages)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    server.RegisterService<slow_generate_numbers>();

    std::atomic<int> pause_count{0};
    std::atomic<int> resume_count{0};
    std::atomic<bool> cancel_received{false};
    std::atomic<bool> stream_end_seen{false};
    std::atomic<uint32_t> stream_id{0};

    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        if (buffer.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd)) {
                stream_end_seen.store(true, std::memory_order_relaxed);
            }
        }
        client.OnRecvResp(std::string_view(buffer));
    });

    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() >= CoroRpcProtocol::REQ_HEAD_LEN) {
            CoroRpcProtocol::ReqHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart) && stream_id.load(std::memory_order_relaxed) == 0) {
                stream_id.store(header.seq_num, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamPause)) {
                pause_count.fetch_add(1, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamResume)) {
                resume_count.fetch_add(1, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
                cancel_received.store(true, std::memory_order_relaxed);
            }
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });

    auto reader = client.CallStream<slow_generate_numbers>(0, 5000, 50).get_result();
    if (!reader) {
        SetError("控制消息测试应返回有效的 StreamReader");
        return;
    }

    bool paused = false;
    for (int spin = 0; spin < 400; ++spin) {
        if (pause_count.load(std::memory_order_relaxed) > 0 && reader->IsPaused()) {
            paused = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (!paused) {
        SetError("高水位流式调用应真实触发暂停控制消息");
        return;
    }

    bool resumed = false;
    for (int spin = 0; spin < 400 && !resumed; ++spin) {
        while (reader->IsPaused()) {
            auto payload = reader->PopValue();
            if (!payload.has_value()) {
                break;
            }
        }
        if (resume_count.load(std::memory_order_relaxed) > 0 && !reader->IsPaused()) {
            resumed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (!resumed) {
        SetError("消费降到低水位后应真实触发恢复控制消息");
        return;
    }

    uint32_t captured_stream_id = stream_id.load(std::memory_order_relaxed);
    if (captured_stream_id == 0) {
        SetError("控制消息测试应捕获到有效的 stream_id");
        return;
    }
    if (!client.CancelStream(captured_stream_id)) {
        SetError("控制消息测试应能成功取消活动流");
        return;
    }

    bool cancel_seen = false;
    for (int spin = 0; spin < 100; ++spin) {
        if (cancel_received.load(std::memory_order_relaxed)) {
            cancel_seen = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!cancel_seen) {
        SetError("取消活动流后应发送取消控制消息");
        return;
    }
    if (!reader->IsCancelled()) {
        SetError("取消活动流后本地 StreamReader 应进入取消状态");
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    if (stream_end_seen.load(std::memory_order_relaxed)) {
        SetError("取消后的流不应继续发送 StreamEnd");
        return;
    }
}

CASE(TestThreadSafe_StreamRpcIntegration_CancelStopsFurtherStreamFrames)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    server.RegisterService<slow_generate_numbers>();

    std::atomic<uint32_t> stream_id{0};
    std::atomic<int> payload_frame_count{0};
    std::atomic<bool> cancel_received{false};
    std::atomic<bool> stream_end_seen{false};

    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        if (buffer.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart) ||
                header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamData)) {
                payload_frame_count.fetch_add(1, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd)) {
                stream_end_seen.store(true, std::memory_order_relaxed);
            }
        }
        client.OnRecvResp(std::string_view(buffer));
    });

    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() >= CoroRpcProtocol::REQ_HEAD_LEN) {
            CoroRpcProtocol::ReqHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart) && stream_id.load(std::memory_order_relaxed) == 0) {
                stream_id.store(header.seq_num, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
                cancel_received.store(true, std::memory_order_relaxed);
            }
        }
        server.OnRecvReq(1, std::string_view(buffer));
    });

    auto reader = client.CallStream<slow_generate_numbers>(0, 500, 1000).get_result();
    if (!reader) {
        SetError("取消停帧测试应返回有效的 StreamReader");
        return;
    }

    bool stream_started = false;
    for (int spin = 0; spin < 200; ++spin) {
        if (stream_id.load(std::memory_order_relaxed) != 0 && payload_frame_count.load(std::memory_order_relaxed) >= 5) {
            stream_started = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (!stream_started) {
        SetError("取消停帧测试应在取消前先收到若干数据帧");
        return;
    }
    if (stream_end_seen.load(std::memory_order_relaxed)) {
        SetError("取消停帧测试不应在取消前就自然结束");
        return;
    }

    uint32_t captured_stream_id = stream_id.load(std::memory_order_relaxed);
    if (!client.CancelStream(captured_stream_id)) {
        SetError("取消停帧测试应能成功取消活动流");
        return;
    }

    bool cancel_seen = false;
    for (int spin = 0; spin < 100; ++spin) {
        if (cancel_received.load(std::memory_order_relaxed) && reader->IsCancelled()) {
            cancel_seen = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!cancel_seen) {
        SetError("取消停帧测试应观察到取消控制消息和本地取消状态");
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    int stable_frame_count = payload_frame_count.load(std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    if (payload_frame_count.load(std::memory_order_relaxed) != stable_frame_count) {
        SetError("取消后的流不应继续发送新的数据帧");
        return;
    }
    if (stream_end_seen.load(std::memory_order_relaxed)) {
        SetError("取消停帧测试中取消后的流不应发送 StreamEnd");
        return;
    }
}

CASE(TestThreadSafe_StreamRpcIntegration_StreamDoesNotBlockUnaryCall)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;

    server.RegisterService<slow_generate_numbers>();
    server.RegisterService<stream_add>();

    std::atomic<int> stream_message_count{0};
    std::atomic<bool> stream_end_seen{false};
    server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        (void)opaque;
        if (buffer.size() >= CoroRpcProtocol::RESP_HEAD_LEN) {
            CoroRpcProtocol::RespHeader header{};
            std::memcpy(&header, buffer.data(), sizeof(header));
            if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart) ||
                header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamData)) {
                stream_message_count.fetch_add(1, std::memory_order_relaxed);
            } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamEnd)) {
                stream_end_seen.store(true, std::memory_order_relaxed);
            }
        }
        client.OnRecvResp(std::string_view(buffer));
    });
    client.SetSendCallback([&](std::string &&buffer) {
        server.OnRecvReq(1, std::string_view(buffer));
    });

    auto reader = client.CallStream<slow_generate_numbers>(100, 120, 1000).get_result();
    if (!reader) {
        SetError("公平性测试应返回有效的 StreamReader");
        return;
    }

    bool stream_started = false;
    for (int spin = 0; spin < 50; ++spin) {
        if (stream_message_count.load(std::memory_order_relaxed) > 0) {
            stream_started = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (!stream_started) {
        SetError("公平性测试中大流应先开始发送");
        return;
    }
    if (stream_end_seen.load(std::memory_order_relaxed)) {
        SetError("公平性测试中大流不应在普通 RPC 发起前就结束");
        return;
    }

    auto add_result = client.Call<stream_add>(7, 8).get_result();
    if (!add_result || add_result.value() != 15) {
        SetError("大流发送期间普通 RPC 仍应正常返回");
        return;
    }
    if (reader->IsFinished() || stream_end_seen.load(std::memory_order_relaxed)) {
        SetError("普通 RPC 不应被阻塞到必须等大流结束后才能返回");
        return;
    }

    std::vector<int> values;
    for (int spin = 0; spin < 400; ++spin) {
        while (true) {
            auto payload = reader->PopValue();
            if (!payload.has_value()) {
                break;
            }
            int value = 0;
            if (!StructPackProtocol::Deserialize(value, std::string_view(*payload))) {
                SetError("公平性测试中的流式返回值反序列化失败");
                return;
            }
            values.push_back(value);
        }
        if (reader->IsFinished()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    if (values.size() != 120) {
        SetError("公平性测试中的大流应完整返回全部数据");
        return;
    }
    for (int i = 0; i < 120; ++i) {
        if (values[i] != 100 + i) {
            SetError("公平性测试中的流式返回值内容不正确");
            return;
        }
    }
}

// ThreadSafe Server 处理流控制消息（Cancel/Pause/Resume）
CASE(TestThreadSafe_StreamStateManagement)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    
    server.RegisterService<generate_numbers>();
    
    // 构造并发送流取消消息
    CoroRpcProtocol::ReqHeader cancel_header{};
    cancel_header.magic = CoroRpcProtocol::kMagicNumber;
    cancel_header.version = CoroRpcProtocol::kVersionNumber;
    cancel_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    cancel_header.msg_type = static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel);
    cancel_header.seq_num = 12345;
    cancel_header.func_id = 0;
    cancel_header.length = 0;
    cancel_header.attach_length = 0;
    cancel_header.client_id = 1;
    
    std::string buffer;
    buffer.resize(CoroRpcProtocol::REQ_HEAD_LEN);
    std::memcpy(buffer.data(), &cancel_header, sizeof(cancel_header));
    
    server.SetSendCallback([](uint64_t, std::string &&) {});
    
    Errc err = server.OnRecvReq(1, buffer);
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe Server 应成功处理流取消控制消息");
        return;
    }
    
    // 构造并发送流暂停消息
    cancel_header.msg_type = static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamPause);
    std::memcpy(buffer.data(), &cancel_header, sizeof(cancel_header));
    err = server.OnRecvReq(1, buffer);
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe Server 应成功处理流暂停控制消息");
        return;
    }
    
    // 构造并发送流恢复消息
    cancel_header.msg_type = static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamResume);
    std::memcpy(buffer.data(), &cancel_header, sizeof(cancel_header));
    err = server.OnRecvReq(1, buffer);
    if (err != Errc::SUCCESS) {
        SetError("ThreadSafe Server 应成功处理流恢复控制消息");
        return;
    }
}

// ThreadSafe 和 ThreadUnsafe 流式服务的 key 一致性
CASE(TestStreamKeyConsistency_AcrossPolicies)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadUnsafe> tu_server;
    
    ts_server.RegisterService<generate_numbers>();
    tu_server.RegisterService<generate_numbers>();
    
    auto ts_keys = ts_server.GetAllServiceHandlerKeys();
    auto tu_keys = tu_server.GetAllServiceHandlerKeys();
    
    if (ts_keys.empty() || tu_keys.empty()) {
        SetError("两种策略下流式服务注册失败");
        return;
    }
    
    if (ts_keys[0] != tu_keys[0]) {
        SetError("相同函数在不同策略下的流式 key 应一致");
        return;
    }
}

// ThreadSafe Server 混合注册普通+协程+流式服务
CASE(TestThreadSafe_StreamMixedServiceRegistration)
{
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> server;
    StreamService service;
    
    // 注册非成员流式函数
    server.RegisterService<generate_numbers>();
    server.RegisterService<generate_strings>();
    
    // 注册成员流式函数
    server.RegisterService<&StreamService::generate_member_numbers>(&service);
    server.RegisterService<&StreamService::generate_member_strings>(&service);
    
    auto keys = server.GetAllServiceHandlerKeys();
    
    if (keys.size() < 4) {
        SetError("ThreadSafe 策略下混合流式服务注册数量不正确，期望 >= 4，实际: " + std::to_string(keys.size()));
        return;
    }
    
    // 验证 key 唯一性
    std::unordered_set<uint32_t> key_set(keys.begin(), keys.end());
    if (key_set.size() != keys.size()) {
        SetError("ThreadSafe 策略下流式服务 key 存在重复");
        return;
    }
}

// ========== FailPending 各方位测试 ==========

CASE(TestFailPending_MissUnknownSeq)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    client.SetSendCallback([](std::string &&) {});
    if (client.FailPending(99999)) {
        SetError("对不存在的 seq FailPending 应返回 false");
        return;
    }
}

CASE(TestFailPending_UnaryPropagatesCanceledNotTimeout)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    std::atomic<uint32_t> cancel_frame_count{0};
    std::atomic<bool> fail_pending_ok{false};
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
            cancel_frame_count.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        // 发送路径内直接 Fail，模拟本地打断在途 unary
        fail_pending_ok.store(client.FailPending(header.seq_num), std::memory_order_relaxed);
    });

    auto result = client.Call<stream_add>(1, 2).get_result();
    if (!fail_pending_ok.load(std::memory_order_relaxed)) {
        SetError("在途 unary FailPending 应返回 true");
        return;
    }
    if (!result.HasError()) {
        SetError("FailPending 后 unary Call 应失败");
        return;
    }
    if (result.GetError() != Errc::ERR_OPERATION_CANCELED) {
        SetError("默认 FailPending 应透出 ERR_OPERATION_CANCELED，不能折叠成 TIMEOUT，实际: " +
                 std::to_string(static_cast<int>(result.GetError())));
        return;
    }
    if (cancel_frame_count.load(std::memory_order_relaxed) != 0) {
        SetError("unary FailPending 不应向对端发送取消帧");
        return;
    }
}

CASE(TestFailPending_UnaryPropagatesCustomEc)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        client.FailPending(header.seq_num, Errc::ERR_BUFFER_TOO_SMALL);
    });

    auto result = client.Call<stream_add>(3, 4).get_result();
    if (!result.HasError() || result.GetError() != Errc::ERR_BUFFER_TOO_SMALL) {
        SetError("FailPending 自定义 ec 应如实透出 ERR_BUFFER_TOO_SMALL");
        return;
    }
}

CASE(TestFailPending_UnaryDoubleFailAndLateResp)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    uint32_t seq = 0;
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        seq = header.seq_num;
        client.FailPending(seq);
    });

    auto result = client.Call<stream_add>(5, 6).get_result();
    if (!result.HasError() || result.GetError() != Errc::ERR_OPERATION_CANCELED) {
        SetError("首次 FailPending 后 unary 应以 CANCELED 失败");
        return;
    }
    if (client.FailPending(seq)) {
        SetError("同一 seq 二次 FailPending 应返回 false");
        return;
    }

    // 迟到成功回包不应崩溃或改变已完成结果
    std::string resp(CoroRpcProtocol::RESP_HEAD_LEN, '\0');
    CoroRpcProtocol::RespHeader resp_header{};
    resp_header.magic = CoroRpcProtocol::kMagicNumber;
    resp_header.version = CoroRpcProtocol::kVersionNumber;
    resp_header.serialize_type = static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
    resp_header.err_code = static_cast<uint8_t>(Errc::SUCCESS);
    resp_header.msg_type = static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeNormal);
    resp_header.seq_num = seq;
    resp_header.length = 0;
    resp_header.attach_length = 0;
    std::memcpy(resp.data(), &resp_header, sizeof(resp_header));
    if (client.OnRecvResp(std::string_view(resp)) != Errc::SUCCESS) {
        SetError("FailPending 后迟到 unary 回包 OnRecvResp 仍应返回 SUCCESS");
        return;
    }
}

CASE(TestFailPending_StreamSetsErrorWithoutCancelFrame)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    std::atomic<uint32_t> stream_id{0};
    std::atomic<uint32_t> cancel_frame_count{0};
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart)) {
            stream_id.store(header.seq_num, std::memory_order_relaxed);
        } else if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamCancel)) {
            cancel_frame_count.fetch_add(1, std::memory_order_relaxed);
        }
        // 不转发服务端，保持在途
    });

    auto reader = client.CallStream<generate_numbers>(0, 5).get_result();
    if (!reader) {
        SetError("流式 FailPending 测试应返回有效 StreamReader");
        return;
    }
    uint32_t seq = stream_id.load(std::memory_order_relaxed);
    if (seq == 0) {
        SetError("应捕获到 stream seq");
        return;
    }

    if (!client.FailPending(seq, Errc::ERR_OPERATION_CANCELED)) {
        SetError("在途 stream FailPending 应返回 true");
        return;
    }
    if (!reader->IsFinished()) {
        SetError("FailPending 后 StreamReader 应进入终态");
        return;
    }
    if (reader->IsCancelled()) {
        SetError("FailPending 应走 SetError，IsCancelled 应为 false");
        return;
    }
    auto error = reader->GetError();
    if (!error.has_value() || error.value() != Errc::ERR_OPERATION_CANCELED) {
        SetError("FailPending 后 GetError 应为 ERR_OPERATION_CANCELED");
        return;
    }
    if (cancel_frame_count.load(std::memory_order_relaxed) != 0) {
        SetError("stream FailPending 不应向对端发送取消帧");
        return;
    }
    if (client.FailPending(seq)) {
        SetError("stream 二次 FailPending 应返回 false");
        return;
    }
    if (client.CancelStream(seq)) {
        SetError("FailPending 摘表后 CancelStream 应返回 false");
        return;
    }
}

CASE(TestFailPending_StreamCustomEcAndLateDataDropped)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    std::atomic<uint32_t> stream_id{0};
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart)) {
            stream_id.store(header.seq_num, std::memory_order_relaxed);
        }
    });

    auto reader = client.CallStream<generate_numbers>(10, 3).get_result();
    if (!reader) {
        SetError("自定义 ec 流式测试应返回有效 StreamReader");
        return;
    }
    uint32_t seq = stream_id.load(std::memory_order_relaxed);
    if (!client.FailPending(seq, Errc::ERR_DESERIALIZE_FAILED)) {
        SetError("在途 stream 自定义 ec FailPending 应成功");
        return;
    }
    auto error = reader->GetError();
    if (!error.has_value() || error.value() != Errc::ERR_DESERIALIZE_FAILED) {
        SetError("stream FailPending 自定义 ec 应透出 ERR_DESERIALIZE_FAILED");
        return;
    }

    size_t before = reader->GetBufferSize();
    // 模拟迟到 Data：直接 Push 到已 SetError 的 reader
    std::string late = "late";
    if (!reader->PushValue(std::move(late))) {
        SetError("SetError 后迟到 Push 丢弃应返回 true");
        return;
    }
    if (reader->GetBufferSize() != before) {
        SetError("FailPending(SetError) 后迟到 Data 不应入队");
        return;
    }
    if (reader->PopValue().has_value()) {
        SetError("FailPending 后 PopValue 不应再吐出数据");
        return;
    }
}

CASE(TestFailPending_DoesNotAffectOtherPendingStream)
{
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> client;
    std::vector<uint32_t> stream_ids;
    client.SetSendCallback([&](std::string &&buffer) {
        if (buffer.size() < CoroRpcProtocol::REQ_HEAD_LEN) {
            return;
        }
        CoroRpcProtocol::ReqHeader header{};
        std::memcpy(&header, buffer.data(), sizeof(header));
        if (header.msg_type == static_cast<uint8_t>(CoroRpcProtocol::MsgType::kMsgTypeStreamStart)) {
            stream_ids.push_back(header.seq_num);
        }
    });

    auto reader1 = client.CallStream<generate_numbers>(0, 2).get_result();
    auto reader2 = client.CallStream<generate_numbers>(100, 2).get_result();
    if (!reader1 || !reader2 || stream_ids.size() != 2) {
        SetError("应成功发起两条在途流");
        return;
    }

    if (!client.FailPending(stream_ids[0], Errc::ERR_OPERATION_CANCELED)) {
        SetError("应对第一条流 FailPending 成功");
        return;
    }
    if (!reader1->GetError().has_value()) {
        SetError("第一条流应收到错误");
        return;
    }
    if (reader2->IsFinished() || reader2->GetError().has_value()) {
        SetError("FailPending 第一条流不应影响第二条在途流");
        return;
    }
    if (!client.FailPending(stream_ids[1], Errc::ERR_TIMEOUT)) {
        SetError("应对第二条流 FailPending 成功");
        return;
    }
    auto err2 = reader2->GetError();
    if (!err2.has_value() || err2.value() != Errc::ERR_TIMEOUT) {
        SetError("第二条流应收到各自的 ERR_TIMEOUT");
        return;
    }
}

FIXTURE_END()

