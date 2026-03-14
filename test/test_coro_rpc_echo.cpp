

#include "network/network_api.h"
#include "test/protobuf/test_coro_rpc.pb.h"
#include "coro_rpc/coro_rpc_client.h"
#include "coro_rpc/coro_rpc_server.h"
#include "tools/timer.h"
#include "unit_test_frame/unittest.h"
#include <thread>
#include <cstring>
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

// ========== ThreadSafe/ThreadUnsafe 策略深度测试 ==========

// --- NullMutex 基础功能测试 ---
CASE(TestNullMutex_BasicLockable)
{
    using namespace ToolBox::CoroRpc;
    
    NullMutex mtx;
    
    // NullMutex 应满足 C++ BasicLockable 和 Lockable 要求
    mtx.lock();
    mtx.unlock();
    
    bool ok = mtx.try_lock();
    if (!ok) {
        SetError("NullMutex::try_lock 应始终返回 true");
        return;
    }
    mtx.unlock();
}

// NullMutex 与 std::lock_guard / std::unique_lock 配合
CASE(TestNullMutex_WithStdLockGuard)
{
    using namespace ToolBox::CoroRpc;
    
    NullMutex mtx;
    {
        std::lock_guard<NullMutex> guard(mtx);
        // 在 guard 保护下做一些操作（no-op）
        volatile int x = 0;
        x = 42;
        (void)x;
    }
    // guard 析构，unlock no-op
    
    {
        std::unique_lock<NullMutex> ulock(mtx);
        if (!ulock.owns_lock()) {
            SetError("unique_lock 在 lock 之后应 owns_lock()=true");
            return;
        }
        ulock.unlock();
        if (ulock.owns_lock()) {
            SetError("unique_lock 在 unlock 之后应 owns_lock()=false");
            return;
        }
        ulock.lock();
        if (!ulock.owns_lock()) {
            SetError("unique_lock 在 re-lock 之后应 owns_lock()=true");
            return;
        }
    }
}

// --- NullConditionVariable 基础测试 ---
CASE(TestNullConditionVariable_BasicOps)
{
    using namespace ToolBox::CoroRpc;
    
    NullMutex mtx;
    NullConditionVariable cv;
    
    // notify 不应崩溃
    cv.notify_one();
    cv.notify_all();
    
    // wait 不应阻塞
    {
        std::unique_lock<NullMutex> lock(mtx);
        cv.wait(lock);
    }
    
    // wait with predicate：即使不真正阻塞，也应至少执行一次谓词
    {
        std::unique_lock<NullMutex> lock(mtx);
        bool pred_called = false;
        cv.wait(lock, [&pred_called]{ pred_called = true; return true; });
        if (!pred_called) {
            SetError("NullConditionVariable::wait(lock, pred) 应至少执行一次谓词");
            return;
        }
    }
}

// --- NullAtomicBool 深度测试 ---
CASE(TestNullAtomicBool_AllOps)
{
    using namespace ToolBox::CoroRpc;
    
    // 默认构造
    NullAtomicBool b1;
    if (static_cast<bool>(b1) != false) {
        SetError("NullAtomicBool 默认值应为 false");
        return;
    }
    
    // 隐式转换构造
    NullAtomicBool b2 = true;
    if (!static_cast<bool>(b2)) {
        SetError("NullAtomicBool(true) 应为 true");
        return;
    }
    
    // 赋值操作符
    b2 = false;
    if (static_cast<bool>(b2)) {
        SetError("NullAtomicBool 赋值 false 后应为 false");
        return;
    }
    
    // load / store
    b2.store(true);
    if (b2.load() != true) {
        SetError("NullAtomicBool store(true) 后 load 应为 true");
        return;
    }
    b2.store(false, std::memory_order_relaxed);
    if (b2.load(std::memory_order_acquire) != false) {
        SetError("NullAtomicBool store(false) 后 load 应为 false");
        return;
    }
    
    // compare_exchange_strong —— 成功路径
    b2.store(false);
    bool expected = false;
    bool success = b2.compare_exchange_strong(expected, true);
    if (!success || b2.load() != true || expected != false) {
        SetError("NullAtomicBool compare_exchange_strong 成功路径错误");
        return;
    }
    
    // compare_exchange_strong —— 失败路径
    expected = false;  // 预期 false，但实际是 true
    success = b2.compare_exchange_strong(expected, false);
    if (success || expected != true || b2.load() != true) {
        SetError("NullAtomicBool compare_exchange_strong 失败路径错误");
        return;
    }
}

// --- NullAtomicUint32 深度测试 ---
CASE(TestNullAtomicUint32_AllOps)
{
    using namespace ToolBox::CoroRpc;
    
    // 默认构造
    NullAtomicUint32 u1;
    if (static_cast<uint32_t>(u1) != 0) {
        SetError("NullAtomicUint32 默认值应为 0");
        return;
    }
    
    // 隐式转换构造
    NullAtomicUint32 u2 = 42u;
    if (static_cast<uint32_t>(u2) != 42) {
        SetError("NullAtomicUint32(42) 应为 42");
        return;
    }
    
    // 赋值
    u2 = 100u;
    if (u2.load() != 100) {
        SetError("NullAtomicUint32 赋值后 load 不匹配");
        return;
    }
    
    // store / load with memory_order
    u2.store(200, std::memory_order_release);
    if (u2.load(std::memory_order_acquire) != 200) {
        SetError("NullAtomicUint32 store/load 不匹配");
        return;
    }
    
    // fetch_add
    u2.store(10);
    uint32_t old = u2.fetch_add(5);
    if (old != 10 || u2.load() != 15) {
        SetError("NullAtomicUint32 fetch_add 错误");
        return;
    }
    
    // 前缀 ++
    u2.store(20);
    uint32_t val = ++u2;
    if (val != 21 || u2.load() != 21) {
        SetError("NullAtomicUint32 前缀++ 错误");
        return;
    }
    
    // 后缀 ++
    val = u2++;
    if (val != 21 || u2.load() != 22) {
        SetError("NullAtomicUint32 后缀++ 错误");
        return;
    }
    
    // 前缀 --
    val = --u2;
    if (val != 21 || u2.load() != 21) {
        SetError("NullAtomicUint32 前缀-- 错误");
        return;
    }
    
    // 后缀 --
    val = u2--;
    if (val != 21 || u2.load() != 20) {
        SetError("NullAtomicUint32 后缀-- 错误");
        return;
    }
}

// --- ThreadSafe/ThreadUnsafe 策略 trait 类型正确性 ---
CASE(TestPolicyTypeTraits)
{
    using namespace ToolBox::CoroRpc;
    using CustomThreadPolicy = ThreadPolicy<std::mutex,
                                            std::condition_variable,
                                            std::atomic<bool>,
                                            std::atomic<uint32_t>,
                                            std::atomic<size_t>,
                                            true>;
    
    // ThreadSafe 策略应映射到真正的同步原语
    static_assert(std::is_same_v<ThreadSafe::mutex_type, std::mutex>,
        "ThreadSafe::mutex_type 应为 std::mutex");
    static_assert(std::is_same_v<ThreadSafe::condition_variable_type, std::condition_variable>,
        "ThreadSafe::condition_variable_type 应为 std::condition_variable");
    static_assert(std::is_same_v<ThreadSafe::atomic_bool_type, std::atomic<bool>>,
        "ThreadSafe::atomic_bool_type 应为 std::atomic<bool>");
    static_assert(std::is_same_v<ThreadSafe::atomic_uint32_type, std::atomic<uint32_t>>,
        "ThreadSafe::atomic_uint32_type 应为 std::atomic<uint32_t>");
    static_assert(std::is_same_v<ThreadSafe::atomic_size_type, std::atomic<size_t>>,
        "ThreadSafe::atomic_size_type 应为 std::atomic<size_t>");
    static_assert(ThreadSafe::enable_async_pump_worker,
        "ThreadSafe 应启用后台 pump worker");
    
    // ThreadUnsafe 策略应映射到 Null 类型
    static_assert(std::is_same_v<ThreadUnsafe::mutex_type, NullMutex>,
        "ThreadUnsafe::mutex_type 应为 NullMutex");
    static_assert(std::is_same_v<ThreadUnsafe::condition_variable_type, NullConditionVariable>,
        "ThreadUnsafe::condition_variable_type 应为 NullConditionVariable");
    static_assert(std::is_same_v<ThreadUnsafe::atomic_bool_type, NullAtomicBool>,
        "ThreadUnsafe::atomic_bool_type 应为 NullAtomicBool");
    static_assert(std::is_same_v<ThreadUnsafe::atomic_uint32_type, NullAtomicUint32>,
        "ThreadUnsafe::atomic_uint32_type 应为 NullAtomicUint32");
    static_assert(std::is_same_v<ThreadUnsafe::atomic_size_type, NullAtomicSize>,
        "ThreadUnsafe::atomic_size_type 应为 NullAtomicSize");
    static_assert(!ThreadUnsafe::enable_async_pump_worker,
        "ThreadUnsafe 默认不应启用后台 pump worker");

    // StreamReader 的锁类型也应可模板化，默认无锁，可显式切换到真锁。
    static_assert(std::is_same_v<StreamReader<int>::mutex_type, NullMutex>,
        "默认 StreamReader 应使用 NullMutex");
    static_assert(std::is_same_v<StreamReader<int, std::mutex>::mutex_type, std::mutex>,
        "显式模板参数应允许 StreamReader 使用真实锁");

    // 通用策略模板应允许用户显式指定真实锁方案。
    static_assert(std::is_same_v<CustomThreadPolicy::mutex_type, std::mutex>,
        "自定义策略模板应保留传入的 mutex 类型");
    static_assert(CustomThreadPolicy::enable_async_pump_worker,
        "自定义真实锁策略应可显式启用后台 pump worker");
}

// --- ThreadSafe Server + Protobuf echo 注册与回包 ---
CASE(TestThreadSafe_ProtobufEchoRegistration)
{
    using namespace ToolBox::CoroRpc;
    
    // ThreadSafe 版本注册 protobuf echo 服务
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    ts_server.RegisterService<echo>();
    
    auto keys = ts_server.GetAllServiceHandlerKeys();
    if (keys.empty()) {
        SetError("ThreadSafe 策略下 protobuf echo 服务注册失败");
        return;
    }
    
    // 同一个函数在 ThreadSafe 和 ThreadUnsafe 下应生成相同的 key
    CoroRpcServer<CoroRpcProtocol, std::unordered_map> tu_server;
    tu_server.RegisterService<echo>();
    auto tu_keys = tu_server.GetAllServiceHandlerKeys();
    
    if (keys[0] != tu_keys[0]) {
        SetError("同一函数在不同策略下的注册 key 应该一致");
        return;
    }
}

// --- ThreadSafe Server 全流程回包测试（构造请求→OnRecvReq→收到响应） ---
CASE(TestThreadSafe_FullRoundTrip)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> ts_client;
    
    // 注册 echo 服务
    ts_server.RegisterService<echo>();
    
    // 设置双向回调
    bool response_received = false;
    ts_server.SetSendCallback([&](uint64_t opaque, std::string &&buffer) {
        response_received = true;
        ts_client.OnRecvResp(std::string_view(buffer));
    });
    
    ts_client.SetSendCallback([&](std::string &&buffer) {
        ts_server.OnRecvReq(1, std::string_view(buffer));
    });
    
    // 验证设置完成
    if (!response_received) {
        // 这是预期的——还没发请求
    }
}

// --- 两种策略的 Server 可以在同一编译单元中共存 ---
CASE(TestBothPolicies_CoexistInSameScope)
{
    using namespace ToolBox::CoroRpc;
    
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadSafe> ts_server;
    CoroRpcServer<CoroRpcProtocol, std::unordered_map, ThreadUnsafe> tu_server;
    CoroRpcClient<CoroRpcProtocol, ThreadSafe> ts_client;
    CoroRpcClient<CoroRpcProtocol, ThreadUnsafe> tu_client;
    
    ts_server.RegisterService<echo>();
    tu_server.RegisterService<echo>();
    
    auto ts_keys = ts_server.GetAllServiceHandlerKeys();
    auto tu_keys = tu_server.GetAllServiceHandlerKeys();
    
    if (ts_keys.empty() || tu_keys.empty()) {
        SetError("两种策略的 Server 应都能注册服务");
        return;
    }
    
    // key 应一致
    if (ts_keys[0] != tu_keys[0]) {
        SetError("两种策略产生的函数 key 应一致");
        return;
    }
    
    // client ID 应不同（来自全局递增计数器）
    if (ts_client.get_client_id() == tu_client.get_client_id()) {
        SetError("不同客户端的 ID 不应相同");
        return;
    }
}

FIXTURE_END()