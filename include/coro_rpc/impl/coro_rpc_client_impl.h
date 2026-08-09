#pragma once

#include <cstdint>
#include <future>
#include <variant>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <utility>
#include <tuple>

#include "tools/cpp20_coroutine.h"
#include "tools/timer.h"
#include "tools/function_traits.h"
#include "protocol/coro_rpc_protocol.h" // IWYU pragma: keep
#include "protocol/serialize_adapter.h"
#include "stream_rpc.h"

namespace ToolBox {
namespace CoroRpc {

template<typename T, auto func>
concept ClientHasGenRegisterKey = requires() {
    T::template GenRegisterKey<func>();
};

// 获取全局唯一的client_id
inline uint64_t get_global_client_id() {
    static std::atomic<uint64_t> client_id{0};
    return client_id.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
struct rpc_return_type{
    using type = T;
};

template<>
struct rpc_return_type<void>{
    using type = std::monostate;
};

template<typename T>
using rpc_async_return_value_t = typename rpc_return_type<typename unwrap_task_return_type<T>::type>::type;

struct resp_body{
    uint8_t serialize_type_;  // 与协议头中的 serialize_type 类型保持一致
    std::string body_buffer_;           // owned storage，防止引用外部 buffer 悬空
    std::string resp_attachment_buf_;   // owned storage，防止引用外部 buffer 悬空

    // 兼容旧代码：允许从 string_view 构造（执行拷贝）
    resp_body() = default;
    resp_body(uint8_t st, std::string_view body, std::string_view att)
        : serialize_type_(st)
        , body_buffer_(body.data(), body.size())
        , resp_attachment_buf_(att.data(), att.size()) {}
    resp_body(uint8_t st, std::string &&body, std::string &&att)
        : serialize_type_(st)
        , body_buffer_(std::move(body))
        , resp_attachment_buf_(std::move(att)) {}
    resp_body(resp_body&&) = default;
    resp_body& operator=(resp_body&&) = default;
    resp_body(const resp_body&) = default;
    resp_body& operator=(const resp_body&) = default;
};

template<typename T>
struct async_rpc_result_value_t{
public:
    async_rpc_result_value_t() = default;
    async_rpc_result_value_t(T &&result, resp_body &&buffer)
    : result_(std::move(result)), buffer_(std::move(buffer)), errc_(CoroRpc::Errc::SUCCESS) {}
    async_rpc_result_value_t(T &&result)
    : result_(std::move(result)), errc_(CoroRpc::Errc::SUCCESS) {}
    // 从错误码构造（Call 失败时使用）
    explicit async_rpc_result_value_t(CoroRpc::Errc ec)
    : errc_(ec) {}

    // 允许直接访问value
    T& operator*() & { return result_; }
    const T& operator*() const & { return result_; }
    T&& operator*() && { return std::move(result_); }
    
    // 允许像指针一样使用
    T* operator->() { return &result_; }
    const T* operator->() const { return &result_; }
    
    // 获取值的引用（替代原来的隐式转换，更安全）
    T& value() & { return result_; }
    const T& value() const & { return result_; }
    T&& value() && { return std::move(result_); }

    // 判断是否有错误 / 获取错误码
    bool HasError() const noexcept { return errc_ != CoroRpc::Errc::SUCCESS; }
    CoroRpc::Errc GetError() const noexcept { return errc_; }

    // operator bool: true = 成功, false = 有错误
    explicit operator bool() const noexcept { return !HasError(); }

    std::string_view GetRespAttachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body ReleaseBuffer() noexcept { return std::move(buffer_); }

private:
    T result_;
    resp_body buffer_;
    CoroRpc::Errc errc_ = CoroRpc::Errc::SUCCESS;
};

template<>
struct async_rpc_result_value_t<void>{
    async_rpc_result_value_t() = default;
    explicit async_rpc_result_value_t(CoroRpc::Errc ec) : errc_(ec) {}
    resp_body buffer_;
    CoroRpc::Errc errc_ = CoroRpc::Errc::SUCCESS;

    bool HasError() const noexcept { return errc_ != CoroRpc::Errc::SUCCESS; }
    CoroRpc::Errc GetError() const noexcept { return errc_; }
    explicit operator bool() const noexcept { return !HasError(); }

    std::string_view GetRespAttachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body ReleaseBuffer() noexcept { return std::move(buffer_); }

};
template<typename T>
using async_rpc_result = std::variant<async_rpc_result_value_t<T>, CoroRpc::Errc>;

template<typename T>
using async_rpc_type_t = typename rpc_return_type<T>::type;

template<typename T>
using rpc_result = std::tuple<T, CoroRpc::Errc>;

template <typename rpc_protocol, typename ThreadSafetyPolicy = ThreadUnsafe>
class CoroRpcClient {
private:
    using mutex_type = typename ThreadSafetyPolicy::mutex_type;
    using atomic_bool_type = typename ThreadSafetyPolicy::atomic_bool_type;
    using atomic_uint32_type = typename ThreadSafetyPolicy::atomic_uint32_type;
    using stream_reader_type = StreamReader<std::string, mutex_type>;

    using PromiseCallback = std::function<void()>;
    using SendCallback = std::function<void(std::string &&)>;
    using rpc_func_key = typename CoroRpcTools::rpc_func_key;
    struct Config {
        uint64_t client_id = get_global_client_id();
        std::chrono::milliseconds Timeout_ = std::chrono::seconds(10);
    };
    struct async_rpc_raw_result_value_type{
        resp_body buffer_;
        uint8_t errc_;
    };
    using async_rpc_raw_result = std::variant<async_rpc_raw_result_value_type, CoroRpc::Errc>;
    struct control_t
    {
        struct handler_t
        {
            ToolBox::HTIMER timer_;
            std::promise<async_rpc_raw_result> promise_;
            PromiseCallback callback_;  // 值语义持有，避免悬空引用
            atomic_bool_type completed_{false};  // 防止超时和正常响应竞争完成
            handler_t(ToolBox::HTIMER && timer,
                        std::promise<async_rpc_raw_result> && promise,
                        PromiseCallback &&callback)
                : timer_(std::move(timer)), promise_(std::move(promise)), callback_(std::move(callback)) {}
            void operator()(resp_body &&buffer, uint8_t rpc_errc) {
                // 防止与 local_error 竞争：只允许完成一次
                bool expected = false;
                if (!completed_.compare_exchange_strong(expected, true)) {
                    return;
                }
                ToolBox::TimerMgr->KillTimer(timer_);
                promise_.set_value(async_rpc_raw_result{async_rpc_raw_result_value_type{std::move(buffer), rpc_errc}});
                if (callback_) callback_();
            }
            void local_error(CoroRpc::Errc ec) {
                // 防止与 operator() 竞争：只允许完成一次
                bool expected = false;
                if (!completed_.compare_exchange_strong(expected, true)) {
                    return;
                }
                RpcLogError("[rpc][client] promise local_error is called, rpc_errc: %d", ec);
                ToolBox::TimerMgr->KillTimer(timer_);
                promise_.set_value(async_rpc_raw_result{ec});
                if (callback_) callback_();
            }
        };
        using handler_ptr = std::shared_ptr<handler_t>;
        // 流式响应处理器：stream_id -> StreamReader
        struct stream_handler_t {
            std::shared_ptr<stream_reader_type> reader;
            uint32_t stream_id;
            uint64_t timeout_timer = 0;  // 超时定时器ID
            std::chrono::steady_clock::time_point start_time;  // 流开始时间
            std::chrono::milliseconds timeout_duration;  // 超时时长
            
            stream_handler_t(uint32_t id, std::chrono::milliseconds timeout)
                : reader(std::make_shared<stream_reader_type>()),
                  stream_id(id),
                  start_time(std::chrono::steady_clock::now()),
                  timeout_duration(timeout) {}
        };
        mutex_type handler_table_mutex_;  // 保护 response_handler_table_ 和 stream_handler_table_
        std::unordered_map<uint32_t, handler_ptr> response_handler_table_;
        std::unordered_map<uint32_t, stream_handler_t> stream_handler_table_;  // stream_id -> stream_handler
        atomic_uint32_type recving_cnt_ = 0;
    };

    static constexpr uint32_t INITIAL_REQUEST_ID = 9527;
    atomic_uint32_type request_id_ = INITIAL_REQUEST_ID;
    std::shared_ptr<control_t> control_;
    std::string req_attachment_storage_;  // owned storage，避免请求附件引用悬空
    Config config_;
    SendCallback send_callback_ = nullptr;
public:
    struct RecvingGuard;
public:

    CoroRpcClient(uint64_t client_id = get_global_client_id())
    : control_(std::make_shared<control_t>())
     {
        config_.client_id = client_id;
    }
    // [KNOWN LIMITATION] 当前析构函数不会显式取消活跃流或清理定时器。
    // 如果 StreamReader 或超时定时器的生命周期超过 client 本身，
    // 回调中捕获的 this/send_callback_ 可能悬空（UAF）。
    // 当前使用模式下 client 始终活过所有 reader 和 timer，因此安全。
    // 若未来改为"client 先析构、reader 后消费"的模式，需要在此处
    // 显式遍历 stream_handler_table_ 并取消所有流/清理定时器。
    ~CoroRpcClient() = default;

    // 获取配置
    const Config &get_config() const { return config_; }

    uint32_t get_client_id() const { return config_.client_id; }
    void set_client_id(uint64_t client_id) { config_.client_id = client_id; }

    // Call RPC with default Timeout_(5s)
    template <auto func, typename... Args>
    auto Call(Args &&...args) -> ToolBox::coro::Task<async_rpc_result_value_t<rpc_async_return_value_t<typename ToolBox::FunctionTraits<decltype(func)>::return_type>>, coro::SharedLooperExecutor> 
    {
        return CallFor_<func>(std::chrono::seconds(5), std::forward<Args>(args)...);
    }
    
    // Call Stream RPC - 返回StreamReader用于接收流式数据
    template <auto func, typename... Args>
    auto CallStream(Args &&...args) -> ToolBox::coro::Task<std::shared_ptr<stream_reader_type>, coro::SharedLooperExecutor>
    {
        return CallStreamFor_<func>(std::chrono::seconds(30), std::forward<Args>(args)...);
    }
    
    // 取消流式RPC
    // stream_id: 流的ID（从StreamReader获取，需要通过其他方式传递）
    // 或者通过StreamReader的Cancel()方法直接取消
    bool CancelStream(uint32_t stream_id) {
        return CancelStream_(stream_id);
    }

    /// 使指定 seq 的在途普通 RPC 或流式 RPC 以 ec 失败并唤醒等待方(不向对端发取消帧)。
    /// 流式路径走 SetError(ec)，调用方应查 GetError()，勿仅依赖 IsCancelled()。
    bool FailPending(uint32_t seq_num,
                     CoroRpc::Errc ec = CoroRpc::Errc::ERR_OPERATION_CANCELED) {
        return FailPending_(seq_num, ec);
    }

    bool SetReqAttachment(std::string_view attachment) {
        if (attachment.size() > UINT32_MAX) {
        RpcLogError("[rpc][client] attachment is too long, max length is UINT32_MAX");
        return false;
        }
        req_attachment_storage_.assign(attachment.data(), attachment.size());
        return true;
    }
    struct RecvingGuard 
    {
        RecvingGuard(control_t *ctrl):ctrl_(ctrl) { ctrl_->recving_cnt_++; }
        ~RecvingGuard() 
        {
            if (ctrl_) {
                ctrl_->recving_cnt_--;
            }
        }
        void release() { ctrl_ = nullptr; }
    private:
        control_t *ctrl_;
    };

    auto SetSendCallback(SendCallback callback)->CoroRpcClient& {
        send_callback_ = std::move(callback);
        return *this;
    }

    Errc OnRecvResp(std::string_view data) {
        return OnRecvResp_(data);
    }

private:
    std::string ConsumeReqAttachment_() {
        std::string attachment = std::move(req_attachment_storage_);
        req_attachment_storage_.clear();
        return attachment;
    }

    // Call RPC with Timeout_
  template <auto func, typename... Args>
  auto CallFor_(auto duration, Args &&...args) 
  -> ToolBox::coro::Task<async_rpc_result_value_t<rpc_async_return_value_t<typename ToolBox::FunctionTraits<decltype(func)>::return_type>>, coro::SharedLooperExecutor> 
  {
        using return_type = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        using result_value_type = async_rpc_result_value_t<rpc_async_return_value_t<return_type>>;
        auto request_attachment = ConsumeReqAttachment_();
        auto async_result = co_await co_await SendRequestForWithAttachment<func, Args...>(
                duration, request_attachment, std::forward<Args>(args)...);
        if (async_result.index() == 0) {
            if constexpr (std::is_void_v<return_type>) {
                co_return result_value_type{};
            } else {
                co_return std::move(std::get<0>(async_result));
            }
        } else {
            // 错误时携带错误码，而不是返回默认空值
            auto ec = std::get<1>(async_result);
            co_return result_value_type{ec};
        }
  }
  
  // Call Stream RPC with Timeout_
  template <auto func, typename... Args>
  auto CallStreamFor_(auto duration, Args &&...args)
  -> ToolBox::coro::Task<std::shared_ptr<stream_reader_type>, coro::SharedLooperExecutor>
  {
        RecvingGuard guard(control_.get());
        uint32_t stream_id = 0;
        auto request_attachment = ConsumeReqAttachment_();

        if (!send_callback_) {
            RpcLogError("[rpc][client] CallStreamFor_: send_callback_ not set");
            co_return nullptr;
        }

        std::string send_buffer;
        auto err = PrepareSendBuffer_<func>(stream_id, request_attachment.size(), send_buffer, true, std::forward<Args>(args)...);
        if (err != CoroRpc::Errc::SUCCESS) {
            RpcLogError("[rpc][client] CallStreamFor_: prepare stream request failed, err: %d", err);
            co_return nullptr;
        }

        if (!request_attachment.empty()) {
            size_t expected_buffer_size = rpc_protocol::REQ_HEAD_LEN + (send_buffer.size() - rpc_protocol::REQ_HEAD_LEN - request_attachment.size()) + request_attachment.size();
            if (send_buffer.size() < expected_buffer_size || send_buffer.size() < request_attachment.size()) {
                RpcLogError("[rpc][client] CallStreamFor_: buffer too small for attachment, buffer size: %zu, attachment size: %zu",
                           send_buffer.size(), request_attachment.size());
                co_return nullptr;
            }
            std::memcpy(send_buffer.data() + send_buffer.size() - request_attachment.size(),
                       request_attachment.data(), request_attachment.size());
        }
        
        // 转换超时时间
        auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        // 创建StreamReader并注册到stream_handler_table_（先注册，再发送，避免同步回环丢首包）
        auto stream_reader = std::make_shared<stream_reader_type>();
        {
            std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
            auto [iter, inserted] = control_->stream_handler_table_.try_emplace(stream_id, stream_id, timeout_ms);
            if (!inserted) {
                RpcLogError("[rpc][client] CallStreamFor_: stream_id conflict, id: %u", stream_id);
                co_return nullptr;
            }
            iter->second.reader = stream_reader;
        }
        
        // 设置流ID和背压回调
        // [KNOWN LIMITATION] 背压回调捕获了裸 this 指针。
        // 只要 client 的生命周期覆盖所有活跃流，这是安全的。
        // 若未来 client 可能先析构，需改为 weak_ptr<control_t> 守护。
        stream_reader->SetStreamId(stream_id);
        stream_reader->SetBackpressureCallback([this](uint32_t id, bool pause) {
            // 发送背压控制消息
            SendStreamControlMessage_(id, pause ? 
                static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamPause) :
                static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamResume));
        });
        
        // 设置超时定时器
        // [KNOWN LIMITATION] 超时回调捕获了裸 this 指针，同上。
        if (timeout_ms.count() > 0) {
            auto timeout_handler = [this, stream_id](int times) {
                (void)times;  // 未使用的参数
                RpcLogWarn("[rpc][client] CallStreamFor_: stream timeout, stream_id: %u", stream_id);
                CancelStream_(stream_id);
            };
            auto timeout_timer_handle = ToolBox::TimerMgr->AddTimer(timeout_handler, timeout_ms.count(), 1, __FILE__, __LINE__);
            {
                std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
                auto it = control_->stream_handler_table_.find(stream_id);
                if (it != control_->stream_handler_table_.end()) {
                    it->second.timeout_timer = timeout_timer_handle;
                }
            }
        }

        send_callback_(std::move(send_buffer));
        
        guard.release();
        co_return stream_reader;
  }

  template <auto func, typename... Args>
  auto SendRequestForWithAttachment(auto time_out_duration,
                                    std::string_view request_attachment,
                                    Args &&...args)
      -> ToolBox::coro::Task<ToolBox::coro::Task<async_rpc_result<rpc_async_return_value_t<typename ToolBox::FunctionTraits<decltype(func)>::return_type>>, coro::SharedLooperExecutor>, coro::SharedLooperExecutor> 
      {
        using return_type = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        using rpc_value_type = rpc_async_return_value_t<return_type>;
        RecvingGuard guard(control_.get());
        uint32_t id = 0;
        std::function<void()> promise_callback;
        typename ToolBox::coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack future_callback = [&promise_callback](std::function<void()> &&handle) {
            promise_callback = std::move(handle);
        };

        std::string send_buffer;
        auto prepare_result = PrepareSendBuffer_<func>(id, request_attachment.size(), send_buffer, false, std::forward<Args>(args)...);
        if (prepare_result != CoroRpc::Errc::SUCCESS) {
            co_return BuildFailedRpcResult_<rpc_value_type>(prepare_result);
        }

        if (!request_attachment.empty())
        {
            size_t expected_buffer_size = rpc_protocol::REQ_HEAD_LEN + (send_buffer.size() - rpc_protocol::REQ_HEAD_LEN - request_attachment.size()) + request_attachment.size();
            if (send_buffer.size() < expected_buffer_size || send_buffer.size() < request_attachment.size()) {
                RpcLogError("[rpc][client] SendRequestForWithAttachment: buffer too small for attachment, buffer size: %zu, attachment size: %zu",
                           send_buffer.size(), request_attachment.size());
                co_return BuildFailedRpcResult_<rpc_value_type>(CoroRpc::Errc::ERR_BUFFER_TOO_SMALL);
            }
            std::memcpy(send_buffer.data() + send_buffer.size() - request_attachment.size(),
                       request_attachment.data(), request_attachment.size());
        }

        if (!send_callback_) {
            RpcLogError("[rpc][client] SendRequestForWithAttachment: send_callback_ not set");
            co_return BuildFailedRpcResult_<rpc_value_type>(CoroRpc::Errc::ERR_SEND_CALLBACK_NOT_SET);
        }

        std::promise<async_rpc_raw_result> promise;
        auto future = promise.get_future();

        std::weak_ptr<control_t> weak_ctrl = control_;
        uint32_t captured_id = id;
        auto timer = ToolBox::TimerMgr->AddTimer([weak_ctrl, captured_id](int times) {
            (void)times;
            auto ctrl = weak_ctrl.lock();
            if (!ctrl)
            {
                RpcLogError("[rpc][client] control is nullptr");
                return;
            }
            RpcLogError("[rpc][client] rpc Timeout_, id: %d", captured_id);
            typename control_t::handler_ptr handler;
            {
                std::lock_guard<mutex_type> lock(ctrl->handler_table_mutex_);
                auto iter = ctrl->response_handler_table_.find(captured_id);
                if(iter == ctrl->response_handler_table_.end())
                {
                    RpcLogError("[rpc][client] response handler table not found, id: %d", captured_id);
                    return;
                }
                handler = iter->second;
                ctrl->response_handler_table_.erase(iter);
                RpcLogDebug("[rpc][client] response handler table erase success, id: %d, table size: %zu", captured_id, ctrl->response_handler_table_.size());
            }
            handler->local_error(CoroRpc::Errc::ERR_TIMEOUT);
        }, (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(time_out_duration).count(), 1, __FILE__, __LINE__);

        auto handler = std::make_shared<typename control_t::handler_t>(std::move(timer), std::move(promise), std::move(promise_callback));
        auto& control = *control_;
        {
            std::lock_guard<mutex_type> lock(control.handler_table_mutex_);
            auto &&[_, is_ok] = control.response_handler_table_.try_emplace(id, handler);
            if (!is_ok) [[unlikely]]
            {
                RpcLogError("[rpc][client] response handler table insert failed, id: {}", id);
                co_return BuildFailedRpcResult_<rpc_value_type>(CoroRpc::Errc::ERR_SERIAL_NUMBER_CONFLICT);
            }
        }

        // 先注册 response handler，再发送请求，避免同步回环时响应先到被丢弃
        send_callback_(std::move(send_buffer));
        guard.release();
        co_return DeserializeRpcResult_<rpc_value_type>(std::move(future), std::move(future_callback), std::weak_ptr<control_t>{control_});
  }

    auto Timeout_(auto duration, std::string error_msg)
    {
        co_return false;
    }

    template<typename T>
    auto HandleResponseBuffer_(int serialize_type, std::string_view buffer, uint8_t Errc) -> rpc_result<T>
    {
        // rpc_return_type<T> ret;
        // CoroRpc::Errc rpc_errc;
        if(Errc == static_cast<uint8_t>(CoroRpc::Errc::SUCCESS))
        {
            if constexpr (std::is_void_v<T>)
            {
                return rpc_result<T>{{}, CoroRpc::Errc::SUCCESS};
            } else {
                auto protocols = rpc_protocol::GetSerializeProtocolByType(serialize_type);
                if(!protocols.has_value())
                {
                    RpcLogError("[CoroRpcServer] OnRecvReq: serialize protocol not supported");
                    return rpc_result<T>{{}, CoroRpc::Errc::ERR_PROTOCOL_NOT_SUPPORTED};
                }
                
                T value;
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> rpc_result<T> {
                    bool is_ok = serialize_protocol::Deserialize(value, buffer);
                    if(!is_ok) [[unlikely]]
                    {
                        RpcLogError("[rpc][client] HandleResponseBuffer_ deserialize failed");
                        return rpc_result<T>{{}, CoroRpc::Errc::ERR_DESERIALIZE_FAILED};
                    }else {
                        return rpc_result<T>{std::move(value), CoroRpc::Errc::SUCCESS};
                    }
                }, protocols.value());
                

            }
        }else {
            return rpc_result<T>{{}, CoroRpc::Errc(Errc)};
        }
        
    }

    template <typename T>
    auto DeserializeRpcResult_(std::future<async_rpc_raw_result> &&future, coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack &&future_callback, std::weak_ptr<control_t> &&ctrl)
        -> ToolBox::coro::Task<async_rpc_result<T>, coro::SharedLooperExecutor> 
    {
        auto result = co_await coro::FutureAwaiter<async_rpc_raw_result>(std::move(future)).with_future_callback(std::move(future_callback));
        if(result.index() == 1) [[unlikely]]
        {
            auto &ret = std::get<1>(result);
            // 如实透出错误原因，避免把 CANCELED 等折叠成 TIMEOUT
            RpcLogError("[rpc][client] DeserializeRpcResult_ error, rpc_errc: %d", ret);
            co_return CoroRpc::Errc(ret);
        }
        auto &ret = std::get<0>(result);
        auto resp_result = HandleResponseBuffer_<T>(ret.buffer_.serialize_type_, ret.buffer_.body_buffer_, ret.errc_);
        if(std::get<1>(resp_result) == CoroRpc::Errc::SUCCESS)
        {
            if constexpr (std::is_void_v<T>)
            {
                co_return {};
            } else {
                co_return async_rpc_result<T>{async_rpc_result_value_t<T>{std::move(std::get<0>(resp_result)), std::move(ret.buffer_)}};
            }
        }
        else {
            RpcLogError("[rpc][client] DeserializeRpcResult_ error, rpc_errc: %d", std::get<1>(resp_result));
            co_return async_rpc_result<T>{async_rpc_result_value_t<T>{std::move(std::get<0>(resp_result))}};
        }
    }

    template <typename T>
    auto BuildFailedRpcResult_(CoroRpc::Errc Errc)
        -> ToolBox::coro::Task<async_rpc_result<T>, coro::SharedLooperExecutor> 
    {
        co_return async_rpc_result<T>{Errc};
    }

    template <auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::SharedLooperExecutor>
    SendRequestForImpl_(auto duration, uint32_t &id, ToolBox::HTIMER &&timer, std::string_view attachment, Args &&...args) 
    {
        // 检查定时器
        int32_t time_left = ToolBox::TimerMgr->GetTimeLeft(timer);
        if(time_left <= 0)
        {
            co_return CoroRpc::Errc::ERR_TIMEOUT;
        }
        co_return co_await SendImpl_<func>(id, attachment, std::forward<Args>(args)...);
    }

    template <auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::SharedLooperExecutor>
    SendImpl_(uint32_t &id, std::string_view attachment, Args &&...args)
    {
        std::string send_buffer;
        auto errc = PrepareSendBuffer_<func>(id, attachment.size(), send_buffer, false, std::forward<Args>(args)...);
        if (errc != CoroRpc::Errc::SUCCESS) {
            RpcLogError("[rpc][client] SendImpl_ PrepareSendBuffer_ failed, rpc_errc: %d", errc);
            co_return errc;
        }
        if (!attachment.empty()) 
        {
            // Append attachment data to send_buffer
            // 验证缓冲区大小：缓冲区应该已经预留了 attachment_size 的空间
            size_t expected_buffer_size = rpc_protocol::REQ_HEAD_LEN + (send_buffer.size() - rpc_protocol::REQ_HEAD_LEN - attachment.size()) + attachment.size();
            if (send_buffer.size() < expected_buffer_size || send_buffer.size() < attachment.size()) {
                RpcLogError("[rpc][client] SendImpl_: buffer too small for attachment, buffer size: %zu, attachment size: %zu", 
                           send_buffer.size(), attachment.size());
                co_return CoroRpc::Errc::ERR_BUFFER_TOO_SMALL;
            }
            // 将 attachment 数据复制到缓冲区末尾（已经预留的空间）
            std::memcpy(send_buffer.data() + send_buffer.size() - attachment.size(), 
                       attachment.data(), attachment.size());
        }
        // RpcLogDebug("[rpc][client] SendImpl_ send_buffer size: %zu, attachment_size: %zu, send_buffer: %s", send_buffer.size(), attachment.size(), send_buffer.c_str());
        if(!send_callback_)
        {
            RpcLogError("[rpc][client] SendImpl_: send_callback_ not set");
            co_return CoroRpc::Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        send_callback_(std::move(send_buffer));
        // send buffer
        co_return CoroRpc::Errc::SUCCESS;
    }
    
    // 发送流式RPC请求
    template <auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::SharedLooperExecutor>
    SendStreamRequestImpl_(uint32_t &id, std::string_view attachment, Args &&...args)
    {
        std::string send_buffer;
        auto errc = PrepareSendBuffer_<func>(id, attachment.size(), send_buffer, true, std::forward<Args>(args)...);
        if (errc != CoroRpc::Errc::SUCCESS) {
            RpcLogError("[rpc][client] SendStreamRequestImpl_ PrepareSendBuffer_ failed, rpc_errc: %d", errc);
            co_return errc;
        }
        if (!attachment.empty()) 
        {
            size_t expected_buffer_size = rpc_protocol::REQ_HEAD_LEN + (send_buffer.size() - rpc_protocol::REQ_HEAD_LEN - attachment.size()) + attachment.size();
            if (send_buffer.size() < expected_buffer_size || send_buffer.size() < attachment.size()) {
                RpcLogError("[rpc][client] SendStreamRequestImpl_: buffer too small for attachment, buffer size: %zu, attachment size: %zu", 
                           send_buffer.size(), attachment.size());
                co_return CoroRpc::Errc::ERR_BUFFER_TOO_SMALL;
            }
            std::memcpy(send_buffer.data() + send_buffer.size() - attachment.size(), 
                       attachment.data(), attachment.size());
        }
        if(!send_callback_)
        {
            RpcLogError("[rpc][client] SendStreamRequestImpl_: send_callback_ not set");
            co_return CoroRpc::Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        send_callback_(std::move(send_buffer));
        co_return CoroRpc::Errc::SUCCESS;
    }


    template<auto func, typename... Args>
    auto PrepareSendBuffer_(uint32_t &id, std::size_t attachment_size, std::string & buffer, bool is_stream, Args &&...args) -> CoroRpc::Errc
    {
        // 编译期计算唯一key（无运行时开销）
        static constexpr rpc_func_key key = [](){
            if constexpr(ClientHasGenRegisterKey<rpc_protocol, func>) {
                return rpc_protocol::template GenRegisterKey<func>();
            } else {
                return CoroRpcTools::AutoGenRegisterKey<func>();
            }
        }();

        // 根据参数类型自动选择合适的序列化协议
        typename rpc_protocol::supported_serialize_protocols serialize_proto = rpc_protocol::template GetSerializeProtocol<func>();

        return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> CoroRpc::Errc {
            using adapter = CoroRpc::SerializeAdapter<func, serialize_protocol>;
            
            // 防御性兼容性检查
            if constexpr (!adapter::is_compatible()) {
                RpcLogError("[rpc][client] Protocol compatibility check failed");
                return CoroRpc::Errc::ERR_SERIALIZE_FAILED;
            }
            
            // 计算序列化大小
            uint32_t body_length = adapter::serialize_size(std::forward<Args>(args)...);
            
            if(body_length > UINT32_MAX) [[unlikely]]
            {
                RpcLogError("[rpc][client] body is too long, max length is %u", UINT32_MAX);
                return CoroRpc::Errc::ERR_MESSAGE_TOO_LARGE;
            }
            
            // 分配缓冲区
            buffer.resize(rpc_protocol::REQ_HEAD_LEN + body_length + attachment_size);
            
            // 初始化请求头（使用 memcpy 写入，避免对齐/别名 UB）
            uint8_t msg_type = is_stream ? static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart) : 0;
            typename rpc_protocol::ReqHeader req_header{};
            req_header.magic = rpc_protocol::kMagicNumber;
            req_header.version = rpc_protocol::kVersionNumber;
            req_header.serialize_type = adapter::get_serialize_type();
            req_header.msg_type = msg_type;
            uint32_t seq_num = ++request_id_;
            req_header.seq_num = seq_num;
            req_header.func_id = key;
            req_header.length = body_length;
            req_header.client_id = config_.client_id;
            req_header.attach_length = static_cast<uint32_t>(attachment_size);
            std::memcpy(buffer.data(), &req_header, sizeof(req_header));
            
            // 保存与报文头一致的序列号
            id = seq_num;
            
            // 执行序列化
            return adapter::serialize_to_buffer(
                buffer.data() + rpc_protocol::REQ_HEAD_LEN,
                buffer.size() - rpc_protocol::REQ_HEAD_LEN - attachment_size,
                std::forward<Args>(args)...);
        }, serialize_proto);
    }

    Errc OnRecvResp_(std::string_view data) {
        typename rpc_protocol::RespHeader header;
        auto err = rpc_protocol::ReadHeader(data, header);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[rpc][client] OnRecvResp_: read header failed, err:%d", err);
            return err;
        }
        std::string_view body;
        std::string_view attachment;
        err = rpc_protocol::ReadPayLoad(header, data.substr(rpc_protocol::RESP_HEAD_LEN), body, attachment);
        if(err != CoroRpc::Errc::SUCCESS)
        {
            RpcLogError("[rpc][client] OnRecvResp_ read payload failed, err:%d", err);
            return err;
        }
        
        // 检查是否是流式响应
        bool is_stream_response = (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart) ||
                                   header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamData) ||
                                   header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamEnd) ||
                                   header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamError));
        
        if (is_stream_response) {
            // 处理流式响应：只在锁内查表/摘表，reader 回调统一放到锁外执行
            std::shared_ptr<stream_reader_type> stream_reader;
            uint64_t timeout_timer = 0;
            bool should_finish = false;
            bool should_set_error = false;
            Errc stream_error = Errc::SUCCESS;
            {
                std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
                auto stream_iter = control_->stream_handler_table_.find(header.seq_num);
                if (stream_iter != control_->stream_handler_table_.end()) {
                    stream_reader = stream_iter->second.reader;
                    if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamEnd)) {
                        should_finish = true;
                        timeout_timer = stream_iter->second.timeout_timer;
                        control_->stream_handler_table_.erase(stream_iter);
                    } else if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamError)) {
                        should_set_error = true;
                        stream_error = static_cast<Errc>(header.err_code);
                        timeout_timer = stream_iter->second.timeout_timer;
                        control_->stream_handler_table_.erase(stream_iter);
                    }
                }
            }
            if (!stream_reader) {
                RpcLogWarn("[rpc][client] OnRecvResp_: stream handler not found for stream_id: %u", header.seq_num);
                return CoroRpc::Errc::SUCCESS;
            }
            if (should_finish) {
                if (timeout_timer != 0) {
                    ToolBox::TimerMgr->KillTimer(timeout_timer);
                }
                stream_reader->Finish();
            } else if (should_set_error) {
                if (timeout_timer != 0) {
                    ToolBox::TimerMgr->KillTimer(timeout_timer);
                }
                stream_reader->SetError(stream_error);
            } else {
                // 流数据 — 拷贝为 owned string，避免引用外部 buffer
                std::string body_str(body.data(), body.size());
                bool within_soft_limit = stream_reader->PushValue(std::move(body_str));
                if (!within_soft_limit) {
                    // 当前 chunk 已保留，仅表示本地缓存达到/超过软上限，背压已经在 PushValue 中处理。
                    RpcLogWarn("[rpc][client] OnRecvResp_: stream buffer soft limit reached, backpressure triggered, stream_id: %u", header.seq_num);
                }
            }
            return CoroRpc::Errc::SUCCESS;
        }
        
        // 普通RPC响应：锁内摘表，锁外执行回调，避免 promise 回调同步重入客户端
        typename control_t::handler_ptr handler;
        {
            std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
            if(auto iter = control_->response_handler_table_.find(header.seq_num); iter != control_->response_handler_table_.end())
            {
                handler = iter->second;
                control_->response_handler_table_.erase(iter);
            }
        }
        if (handler) {
            (*handler)(resp_body(header.serialize_type, std::string(body), std::string(attachment)), header.err_code);
        }
        return CoroRpc::Errc::SUCCESS;
    }
    
    // 取消流（私有函数）
    bool CancelStream_(uint32_t stream_id) {
        std::shared_ptr<stream_reader_type> stream_reader;
        uint64_t timeout_timer = 0;
        {
            std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
            auto iter = control_->stream_handler_table_.find(stream_id);
            if (iter == control_->stream_handler_table_.end()) {
                return false;
            }
            stream_reader = iter->second.reader;
            timeout_timer = iter->second.timeout_timer;
            control_->stream_handler_table_.erase(iter);
        }

        if (stream_reader) {
            stream_reader->Cancel();
        }
        if (timeout_timer != 0) {
            ToolBox::TimerMgr->KillTimer(timeout_timer);
        }

        // 发送取消消息到服务器
        SendStreamControlMessage_(stream_id, static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamCancel));
        return true;
    }

    bool FailPending_(uint32_t seq_num, CoroRpc::Errc ec) {
        typename control_t::handler_ptr handler;
        std::shared_ptr<stream_reader_type> stream_reader;
        uint64_t timeout_timer = 0;
        {
            std::lock_guard<mutex_type> lock(control_->handler_table_mutex_);
            auto unary_iter = control_->response_handler_table_.find(seq_num);
            if (unary_iter != control_->response_handler_table_.end()) {
                handler = unary_iter->second;
                control_->response_handler_table_.erase(unary_iter);
            } else {
                auto stream_iter = control_->stream_handler_table_.find(seq_num);
                if (stream_iter != control_->stream_handler_table_.end()) {
                    stream_reader = stream_iter->second.reader;
                    timeout_timer = stream_iter->second.timeout_timer;
                    control_->stream_handler_table_.erase(stream_iter);
                }
            }
        }

        if (handler) {
            handler->local_error(ec);
            RpcLogWarn("[rpc][client] FailPending_: unary seq=%u ec=%d",
                       seq_num, static_cast<int>(ec));
            return true;
        }
        if (stream_reader) {
            if (timeout_timer != 0) {
                ToolBox::TimerMgr->KillTimer(timeout_timer);
            }
            // 走 SetError 透出 ec；与 CancelStream(Cancel+发对端帧) 语义不同
            stream_reader->SetError(ec);
            RpcLogWarn("[rpc][client] FailPending_: stream seq=%u ec=%d",
                       seq_num, static_cast<int>(ec));
            return true;
        }

        RpcLogWarn("[rpc][client] FailPending_: seq not found, seq=%u ec=%d",
                   seq_num, static_cast<int>(ec));
        return false;
    }
    
    // 发送流控制消息（暂停/恢复/取消）
    void SendStreamControlMessage_(uint32_t stream_id, uint8_t msg_type) {
        if (!send_callback_) {
            RpcLogError("[rpc][client] SendStreamControlMessage_: send_callback_ not set");
            return;
        }
        
        // 构造控制消息（只有头部，没有body）
        std::string control_buf;
        control_buf.resize(rpc_protocol::REQ_HEAD_LEN);
        
        // 使用 memcpy 写入协议头，避免对齐/别名 UB
        typename rpc_protocol::ReqHeader req_header{};
        req_header.magic = rpc_protocol::kMagicNumber;
        req_header.version = rpc_protocol::kVersionNumber;
        req_header.serialize_type = static_cast<uint8_t>(rpc_protocol::SerializeType::kSerializeTypeStruct);
        req_header.msg_type = msg_type;
        req_header.seq_num = stream_id;  // 使用stream_id作为seq_num
        req_header.func_id = 0;  // 控制消息不需要func_id
        req_header.length = 0;  // 没有body
        req_header.attach_length = 0;
        req_header.client_id = config_.client_id;
        std::memcpy(control_buf.data(), &req_header, sizeof(req_header));
        
        send_callback_(std::move(control_buf));
    }

private:
    


};  // class CoroRpcClient

}; // namespace CoroRpc
}; // namespace ToolBox

