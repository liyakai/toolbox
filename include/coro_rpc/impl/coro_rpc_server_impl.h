#pragma once

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "coro_rpc_def_interenal.h"
#include "protocol/coro_rpc_protocol.h"
#include "protocol/serialize_adapter.h"
#include "stream_rpc.h"
#include "tools/cpp20_coroutine.h"
#include "tools/function_traits.h"

namespace ToolBox {
namespace CoroRpc {

template<typename T, auto func>
concept ServerHasGenRegisterKey = requires() {
    T::template GenRegisterKey<func>();
};

// 检测是否是StreamGenerator类型
template<typename T>
struct IsStreamGenerator : std::false_type {};

template<typename T>
struct IsStreamGenerator<StreamGenerator<T>> : std::true_type {};

template<typename T>
inline constexpr bool kIsStreamGeneratorV = IsStreamGenerator<T>::value;

template <typename rpc_protocol, template<typename...> typename map_t, typename ThreadSafetyPolicy>
class CoroRpcServer {
private:
    using mutex_type = typename ThreadSafetyPolicy::mutex_type;
    using condition_variable_type = typename ThreadSafetyPolicy::condition_variable_type;
    using atomic_bool_type = typename ThreadSafetyPolicy::atomic_bool_type;
    using atomic_size_type = typename ThreadSafetyPolicy::atomic_size_type;

    struct RequestContext;
    using request_context_ptr = std::shared_ptr<RequestContext>;

    using handler_t = std::function<std::pair<Errc, std::string>(std::string_view, std::string_view, typename rpc_protocol::supported_serialize_protocols)>;
    using core_handler_t = std::function<ToolBox::coro::Task<std::pair<Errc, std::string>>(std::string, std::string, typename rpc_protocol::supported_serialize_protocols protocols, request_context_ptr)>;
    // 流式处理器类型：返回StreamGenerator<std::string>（已序列化的数据）
    using stream_handler_t = std::function<StreamGenerator<std::string>(std::string, std::string, typename rpc_protocol::supported_serialize_protocols protocols)>;
    using rpc_func_key = typename CoroRpcTools::rpc_func_key_t;
    using SendCallback = std::function<void(uint64_t opaque, std::string &&)>;

    SendCallback send_callback_ = nullptr;
    map_t<rpc_func_key, handler_t> rpc_server_handler_map_;
    map_t<rpc_func_key, core_handler_t> rpc_server_core_handler_map_;
    // 流式处理器映射
    map_t<rpc_func_key, stream_handler_t> stream_handler_map_;
    map_t<rpc_func_key, std::string> func_key2name_map_;
    map_t<std::string, rpc_func_key> name2func_key_map_;

    // ---------- per-request context ----------
    // 请求级上下文：每次 OnRecvReq 时创建一份 owned snapshot，避免 attachment 指向外部 buffer
    struct RequestContext {
        std::string req_attachment;      // 当前请求的 attachment（owned，避免悬空）
        std::string resp_attachment;     // handler 设置的响应附件（owned，避免悬空）
        bool has_resp_attachment = false;
    };
    static inline thread_local RequestContext* current_request_ctx_ = nullptr;

    static void InstallRequestContext_(void* ctx) noexcept {
        current_request_ctx_ = static_cast<RequestContext*>(ctx);
    }

    static ToolBox::coro::ResumeContext MakeResumeContext_(const request_context_ptr& ctx) noexcept {
        return ToolBox::coro::ResumeContext{ctx.get(), &CoroRpcServer::InstallRequestContext_};
    }

    // WARNING: RequestContextBinder_ 使用 RAII 管理 thread_local 请求上下文，
    // 同时把上下文挂到 ToolBox::coro 的恢复链路上，使当前请求内创建/恢复的 Task
    // 能自动重新安装对应 request context。
    class RequestContextBinder_ {
    public:
        explicit RequestContextBinder_(const request_context_ptr& ctx)
            : resume_scope_(MakeResumeContext_(ctx)) {}
    private:
        ToolBox::coro::ResumeContextScope resume_scope_;
    };

    static request_context_ptr CreateRequestContext_(std::string_view attachment) {
        auto ctx = std::make_shared<RequestContext>();
        ctx->req_attachment.assign(attachment.data(), attachment.size());
        return ctx;
    }

    // 兼容旧接口：函数级全局 resp_attachment 回调（优先级低于 per-request 版本）
    map_t<rpc_func_key, std::function<std::string_view()>> resp_attachment_func_map_;
    
    // 托管 fire-and-forget 的 Task，防止协程 frame 泄漏
    mutex_type detached_tasks_mutex_;
    std::list<ToolBox::coro::Task<std::pair<Errc, std::string>>> detached_coro_tasks_;
    // 已完成的 coro task 迭代器（惰性 GC：回调中只标记，SweepDetachedTasks_ 统一 erase）
    std::vector<typename std::list<ToolBox::coro::Task<std::pair<Errc, std::string>>>::iterator> finished_coro_tasks_;
    atomic_size_type outstanding_detached_tasks_{0};
    mutex_type detached_shutdown_mutex_;
    condition_variable_type detached_shutdown_cv_;

    // 统一清理所有已标记完成的 detached task
    // 调用时机：OnRecvReq_ 入口、析构
    // 此时协程已经走过 final_suspend (suspend_always)，h.done()==true，
    // ~Task() 可以正确调用 h.destroy()
    void SweepDetachedTasks_() {
        std::lock_guard<mutex_type> lock(detached_tasks_mutex_);
        for (auto& it : finished_coro_tasks_) {
            detached_coro_tasks_.erase(it);
        }
        finished_coro_tasks_.clear();
    }

    template<typename OnComplete>
    void DetachCoroTask_(ToolBox::coro::Task<std::pair<Errc, std::string>>&& task, OnComplete&& on_complete) {
        outstanding_detached_tasks_.fetch_add(1, std::memory_order_relaxed);
        // 第一步：在锁内把 task 挂到列表上，拿到迭代器
        typename std::list<ToolBox::coro::Task<std::pair<Errc, std::string>>>::iterator it;
        {
            std::lock_guard<mutex_type> lock(detached_tasks_mutex_);
            detached_coro_tasks_.push_back(std::move(task));
            it = std::prev(detached_coro_tasks_.end());
        }
        // 第二步：在锁外注册回调，避免 task 已完成时同步回调造成自锁
        it->then([on_complete = std::forward<OnComplete>(on_complete)](std::pair<Errc, std::string> result) mutable {
            on_complete(std::move(result));
        });
        // 只标记完成，不立即 erase（此时 h.done() 可能还是 false）
        it->finally([this, it]() {
            {
                std::lock_guard<mutex_type> lock2(detached_tasks_mutex_);
                finished_coro_tasks_.push_back(it);
            }
            outstanding_detached_tasks_.fetch_sub(1, std::memory_order_relaxed);
            detached_shutdown_cv_.notify_all();
        });
    }
    
    // ====== 流式 RPC 状态机 ======
    // 替代原来的 StreamState + cv.wait + detached_stream_tasks_ 协程方案。
    // 改为事件驱动的分段发送：PumpStream_() 在锁外推进 generator，并用 pumping 标志防止重入。
    //
    // [KNOWN LIMITATION] 流键当前仅使用 seq_num（uint32_t），不包含 client_id。
    // 如果未来需要多个 CoroRpcClient 共享同一个 CoroRpcServer 实例，
    // 不同客户端的 seq_num 可能重叠，导致串流。
    // 届时需要将流键升级为 (client_id, seq_num) 复合键。
    // 目前每个连接使用独立的 server 实例，此问题不存在。

    struct ActiveStream_ {
        StreamGenerator<std::string> generator;
        request_context_ptr request_ctx;
        uint64_t opaque = 0;
        rpc_func_key key = 0;
        typename rpc_protocol::ReqHeader req_header;
        bool started = false;       // 是否已发过 StreamStart
        bool paused = false;        // 背压暂停
        bool cancelled = false;     // 已取消
        bool finished = false;      // 生成器已耗尽或发送完毕
        bool pumping = false;       // 是否已有 pump 正在推进，避免同步重入
    };
    using active_stream_ptr = std::shared_ptr<ActiveStream_>;
    mutex_type stream_state_mutex_;  // 保护流状态表的互斥锁
    map_t<uint32_t, active_stream_ptr> active_streams_;  // stream_id -> ActiveStream_

    // 单次 pump 的批量阈值：达到阈值后必须真正跳出当前调用栈，避免霸占请求/IO 线程。
    static constexpr size_t kMaxChunksPerPump = 64;
    static constexpr bool kUseAsyncPumpWorker_ = ThreadSafetyPolicy::enable_async_pump_worker;

    mutex_type pump_queue_mutex_;
    condition_variable_type pump_queue_cv_;
    std::deque<std::pair<uint32_t, active_stream_ptr>> pending_pump_queue_;
    atomic_bool_type stop_pump_worker_{false};
    std::thread pump_worker_;

    void PumpWorkerLoop_() {
        while (true) {
            std::pair<uint32_t, active_stream_ptr> next_job;
            {
                std::unique_lock<mutex_type> lock(pump_queue_mutex_);
                pump_queue_cv_.wait(lock, [this]() {
                    return stop_pump_worker_.load(std::memory_order_relaxed) || !pending_pump_queue_.empty();
                });
                if (stop_pump_worker_.load(std::memory_order_relaxed) && pending_pump_queue_.empty()) {
                    return;
                }
                next_job = std::move(pending_pump_queue_.front());
                pending_pump_queue_.pop_front();
            }
            PumpStream_(next_job.first, std::move(next_job.second));
        }
    }

    void StopPumpWorker_() {
        if constexpr (kUseAsyncPumpWorker_) {
            stop_pump_worker_.store(true, std::memory_order_relaxed);
            {
                std::lock_guard<mutex_type> lock(pump_queue_mutex_);
                pending_pump_queue_.clear();
            }
            pump_queue_cv_.notify_all();
            if (pump_worker_.joinable()) {
                pump_worker_.join();
            }
        }
        // TODO [FUTURE] 停机时应遍历 active_streams_ 向未完成流的客户端发送
        // StreamError(ERR_OPERATION_CANCELED)，让客户端能区分"正常结束"和"服务端中断"。
        // 当前客户端只能靠超时发现流中断。
        {
            std::lock_guard<mutex_type> lock(stream_state_mutex_);
            if (!active_streams_.empty()) {
                RpcLogWarn("[CoroRpcServer] StopPumpWorker_: %zu active streams will be silently dropped during shutdown", active_streams_.size());
            }
            active_streams_.clear();
        }
    }

    void SchedulePumpStream_(uint32_t stream_id, active_stream_ptr stream) {
        if constexpr (kUseAsyncPumpWorker_) {
            if (stop_pump_worker_.load(std::memory_order_relaxed)) {
                return;
            }
            {
                std::lock_guard<mutex_type> lock(pump_queue_mutex_);
                pending_pump_queue_.emplace_back(stream_id, std::move(stream));
            }
            pump_queue_cv_.notify_one();
        } else {
            PumpStream_(stream_id, std::move(stream));
        }
    }

    bool TryStartPump_(uint32_t stream_id, active_stream_ptr& stream) {
        std::lock_guard<mutex_type> lock(stream_state_mutex_);
        auto iter = active_streams_.find(stream_id);
        if (iter == active_streams_.end()) {
            return false;
        }
        stream = iter->second;
        if (!stream || stream->pumping || stream->paused || stream->cancelled || stream->finished) {
            return false;
        }
        stream->pumping = true;
        return true;
    }

    bool RemoveActiveStream_(uint32_t stream_id, const active_stream_ptr& stream) {
        std::lock_guard<mutex_type> lock(stream_state_mutex_);
        auto iter = active_streams_.find(stream_id);
        if (iter == active_streams_.end() || !stream || iter->second.get() != stream.get()) {
            return false;
        }
        stream->pumping = false;
        stream->finished = true;
        active_streams_.erase(iter);
        return true;
    }

    // 锁外推进流，避免 send_callback_ 同步重入时与 stream_state_mutex_ 形成自锁。
    void PumpStream_(uint32_t stream_id, active_stream_ptr stream) {
        while (true) {
            {
                std::lock_guard<mutex_type> lock(stream_state_mutex_);
                auto iter = active_streams_.find(stream_id);
                if (iter == active_streams_.end() || !stream || iter->second.get() != stream.get()) {
                    return;
                }
                if constexpr (kUseAsyncPumpWorker_) {
                    if (stop_pump_worker_.load(std::memory_order_relaxed)) {
                        stream->pumping = false;
                        stream->finished = true;
                        active_streams_.erase(iter);
                        return;
                    }
                }
                if (stream->cancelled) {
                    RpcLogInfo("[CoroRpcServer] PumpStream_: stream cancelled, stream_id: %u", stream_id);
                    stream->pumping = false;
                    stream->finished = true;
                    active_streams_.erase(iter);
                    return;
                }
                if (stream->paused || stream->finished) {
                    stream->pumping = false;
                    return;
                }
            }

            size_t chunks_sent = 0;
            while (chunks_sent < kMaxChunksPerPump) {
                {
                    std::lock_guard<mutex_type> lock(stream_state_mutex_);
                    auto iter = active_streams_.find(stream_id);
                    if (iter == active_streams_.end() || !stream || iter->second.get() != stream.get()) {
                        return;
                    }
                    if constexpr (kUseAsyncPumpWorker_) {
                        if (stop_pump_worker_.load(std::memory_order_relaxed)) {
                            stream->pumping = false;
                            stream->finished = true;
                            active_streams_.erase(iter);
                            return;
                        }
                    }
                    if (stream->cancelled) {
                        RpcLogInfo("[CoroRpcServer] PumpStream_: stream cancelled during pump, stream_id: %u", stream_id);
                        stream->pumping = false;
                        stream->finished = true;
                        active_streams_.erase(iter);
                        return;
                    }
                    if (stream->paused || stream->finished) {
                        stream->pumping = false;
                        return;
                    }
                }

                bool has_next = false;
                try {
                    RequestContextBinder_ request_ctx_guard(stream->request_ctx);
                    has_next = stream->generator.Next();
                } catch (const std::exception &e) {
                    RpcLogError("[CoroRpcServer] PumpStream_: generator exception: %s, stream_id: %u", e.what(), stream_id);
                    if (RemoveActiveStream_(stream_id, stream)) {
                        SendStreamResponse_(stream->opaque, stream->key, stream->req_header, "",
                            static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamError), Errc::ERR_HANDLER_THROW_EXCEPTION);
                    }
                    return;
                } catch (...) {
                    RpcLogError("[CoroRpcServer] PumpStream_: generator unknown exception, stream_id: %u", stream_id);
                    if (RemoveActiveStream_(stream_id, stream)) {
                        SendStreamResponse_(stream->opaque, stream->key, stream->req_header, "",
                            static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamError), Errc::ERR_HANDLER_THROW_EXCEPTION);
                    }
                    return;
                }

                if (!has_next) {
                    bool should_send_end = false;
                    {
                        std::lock_guard<mutex_type> lock(stream_state_mutex_);
                        auto iter = active_streams_.find(stream_id);
                        if (iter == active_streams_.end() || !stream || iter->second.get() != stream.get()) {
                            return;
                        }
                        if (!stream->cancelled) {
                            stream->pumping = false;
                            stream->finished = true;
                            active_streams_.erase(iter);
                            should_send_end = true;
                        } else {
                            stream->pumping = false;
                            stream->finished = true;
                            active_streams_.erase(iter);
                        }
                    }
                    if (should_send_end) {
                        SendStreamResponse_(stream->opaque, stream->key, stream->req_header, "",
                            static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamEnd));
                    }
                    return;
                }

                std::string data = std::move(stream->generator.value());
                uint8_t msg_type = static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamData);
                {
                    std::lock_guard<mutex_type> lock(stream_state_mutex_);
                    auto iter = active_streams_.find(stream_id);
                    if (iter == active_streams_.end() || !stream || iter->second.get() != stream.get()) {
                        return;
                    }
                    if (stream->cancelled) {
                        RpcLogInfo("[CoroRpcServer] PumpStream_: stream cancelled before send, stream_id: %u", stream_id);
                        stream->pumping = false;
                        stream->finished = true;
                        active_streams_.erase(iter);
                        return;
                    }
                    if (!stream->started) {
                        msg_type = static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart);
                        stream->started = true;
                    }
                }

                Errc err = SendStreamResponse_(stream->opaque, stream->key, stream->req_header, std::move(data), msg_type);
                if (err != Errc::SUCCESS) {
                    RpcLogError("[CoroRpcServer] PumpStream_: send failed, err: %d, stream_id: %u", err, stream_id);
                    RemoveActiveStream_(stream_id, stream);
                    return;
                }
                ++chunks_sent;
            }

            if constexpr (kUseAsyncPumpWorker_) {
                // 关键修复：真正 return，把后续 batch 交给后台队列，而不是在当前调用栈里忙让 CPU。
                SchedulePumpStream_(stream_id, stream);
                return;
            } else {
                // ThreadUnsafe 模式仍保持历史上的同步推进语义。
                std::this_thread::yield();
            }
        }
    }

public:
    CoroRpcServer() {
        if constexpr (kUseAsyncPumpWorker_) {
            pump_worker_ = std::thread([this]() {
                PumpWorkerLoop_();
            });
        }
    }
    ~CoroRpcServer() {
        StopPumpWorker_();
        {
            std::unique_lock<mutex_type> lock(detached_shutdown_mutex_);
            detached_shutdown_cv_.wait(lock, [this]() {
                return outstanding_detached_tasks_.load(std::memory_order_relaxed) == 0;
            });
        }
        // 析构前清理所有已完成的 detached coro tasks
        SweepDetachedTasks_();
        // active_streams_ 中剩余的流在 map 析构时自动清理（shared_ptr + StreamGenerator::~析构会 destroy 协程）
    }

    /*
    * @brief Register RPC service functions(member function)
    */
    template <auto first, auto... functions>
    void RegisterService(typename ToolBox::FunctionTraits<decltype(first)>::class_type *self) {
        RegisterOneHandler<first>(self);
        (RegisterOneHandler<functions>(self), ...);
    }

    template <auto first>
    void RegisterService(typename ToolBox::FunctionTraits<decltype(first)>::class_type *self, const auto &key) {
        RegistOneHandlerImpl<first>(self, key);
    }

    /*
    * @brief Register RPC service functions (non-member function)
    */
    template <auto first, auto... func>
    void RegisterService() {
        RegisterOneHandler<first>();
        (RegisterOneHandler<func>(), ...);
    }

    template <auto func>
    void RegisterService(const auto &key) {
        RegistOneHandlerImpl<func>(key);
    }

    const std::string& GetFunctionKeyName(const rpc_func_key &key) const {
        static std::string empty_str;
        auto it = func_key2name_map_.find(key);
        if (it == func_key2name_map_.end()) {
            return empty_str;
        }
        return it->second;
    }

    Errc OnRecvReq(uint64_t opaque, std::string_view data) {
        return OnRecvReq_(opaque, data);
    }
    Errc SetSendCallback(SendCallback &&callback) {
        send_callback_ = std::move(callback);
        return Errc::SUCCESS;
    }
    template <auto func>
    void SetRespAttachmentFunc(std::function<std::string_view()> &&resp_attachment_func) {
        static const rpc_func_key key = [] {
            if constexpr (ServerHasGenRegisterKey<rpc_protocol, func>) {
                return rpc_protocol::template GenRegisterKey<func>();
            } else {
                return CoroRpcTools::AutoGenRegisterKey<func>();
            }
        }();

        // 每次调用时，用 key 在当前 this 的 map 上访问
        resp_attachment_func_map_[key] = std::move(resp_attachment_func);
    }
    
    /**
     * @brief 获取当前请求的附件数据（请求级安全）
     * @note  返回当前绑定请求上下文里的 owned attachment；若当前没有活跃请求，则返回空。
     * @warning 当前实现会为库内 `ToolBox::coro::Task`、回调 awaitor 和 future awaitor
     *          传播请求上下文；若用户引入第三方 awaitable 并直接恢复 coroutine handle，
     *          则需在其恢复点接入同样的 ResumeContext 绑定逻辑。
     */
    std::string_view GetReqAttachment() const noexcept {
        if (!current_request_ctx_) {
            return {};
        }
        return current_request_ctx_->req_attachment;
    }

    /**
     * @brief 设置当前请求的响应附件（请求级，handler 函数体内调用）
     * @note  此方法设置的 resp_attachment 仅对当前请求生效，优先级高于 SetRespAttachmentFunc。
     *        若当前没有活跃请求，则忽略本次设置，避免把"游离附件"泄露到后续请求。
     * @warning 同 GetReqAttachment()，若使用第三方 awaitable，需确保其恢复点也安装请求上下文。
     */
    void SetRespAttachment(std::string_view attachment) {
        if (!current_request_ctx_) {
            return;
        }
        current_request_ctx_->resp_attachment.assign(attachment.data(), attachment.size());
        current_request_ctx_->has_resp_attachment = true;
    }

    std::vector<rpc_func_key> GetAllServiceHandlerKeys()
    {
        std::vector<rpc_func_key> keys;
        keys.reserve(rpc_server_handler_map_.size() + rpc_server_core_handler_map_.size());
        for(auto &iter : rpc_server_handler_map_)
        {
            keys.push_back(iter.first);
        }
        for(auto &iter : rpc_server_core_handler_map_)
        {
            keys.push_back(iter.first);
        }
        for(auto &iter : stream_handler_map_)
        {
            keys.push_back(iter.first);
        }

        return keys;
    }
private:
    template <auto func, typename Self>
    void RegisterOneHandler(Self *self)
    {
        if (nullptr == self) [[unlikely]]
        {
            RpcLogError("[CoroRpcServer] RegisterOneHandler: self is nullptr");
            return;
        }
        rpc_func_key key{};
        if constexpr(ServerHasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }

        RpcLogDebug("[rpc][server] RegisterOneHandler.class: key: %u, func: %.*s", key, static_cast<int>(ToolBox::GetFuncName<func>().size()), ToolBox::GetFuncName<func>().data());

        // 自动填充函数名映射，用于日志和可观测性
        {
            constexpr auto fname = ToolBox::GetFuncNameWithClass<func>();
            func_key2name_map_[key] = std::string(fname.data(), fname.size());
            name2func_key_map_[std::string(fname.data(), fname.size())] = key;
        }

        RegistOneHandlerImpl<func>(self,key);
    }


    template<auto func, typename Self>
    void RegistOneHandlerImpl(Self *self, const rpc_func_key &key)
    {
        if (self == nullptr) [[unlikely]]
        {
            RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: self is nullptr");
            return;
        }
        using ReturnType = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        constexpr auto name = ToolBox::GetFuncNameWithClass<func>();

        RpcLogInfo("[CoroRpcServer] RegistOneHandlerImpl: key: %u, func: %.*s",
                   key, static_cast<int>(name.size()), name.data());

        // 检查是否是流式RPC（返回StreamGenerator）
        if constexpr (kIsStreamGeneratorV<ReturnType>)
        {
            // 流式RPC处理
            using StreamValueType = typename ReturnType::value_type;
            auto iter = stream_handler_map_.emplace(key, [&, self](std::string data, std::string attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> StreamGenerator<std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> StreamGenerator<std::string> {
                    if constexpr (std::is_void_v<typename ToolBox::FunctionTraits<decltype(func)>::class_type>) {
                        return ExecuteStream_<serialize_protocol, func, StreamValueType>(std::move(data), std::move(attachment), nullptr);
                    } else {
                        return ExecuteStream_<serialize_protocol, func, StreamValueType>(std::move(data), std::move(attachment), self);
                    }
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: stream handler key already exists");
                return;
            }
            RpcLogInfo("[CoroRpcServer] Registered stream handler for key: %u", key);
        }
        else if constexpr(ToolBox::is_specialization_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string data, std::string attachment, typename rpc_protocol::supported_serialize_protocols protocols, request_context_ptr request_ctx) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                    return ExecuteCoro_<serialize_protocol, func>(std::move(data), std::move(attachment), request_ctx, self);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    // Execute_ 内部已经有兼容性检查，这里可以移除冗余检查
                    return Execute_<serialize_protocol, func>(data, attachment, self);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        }
    }

    template <auto func>
    void RegisterOneHandler()
    {
        rpc_func_key key{};
        if constexpr(ServerHasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }
        RpcLogDebug("[rpc][server] RegisterOneHandler.normal: key: %u, func: %s", key, ToolBox::GetFuncName<func>().data());

        // 自动填充函数名映射，用于日志和可观测性
        {
            constexpr auto fname = ToolBox::GetFuncName<func>();
            func_key2name_map_[key] = std::string(fname.data(), fname.size());
            name2func_key_map_[std::string(fname.data(), fname.size())] = key;
        }

        RegistOneHandlerImpl<func>(key);
    }

    template<auto func>
    void RegistOneHandlerImpl(const rpc_func_key &key)
    {
        static_assert(!std::is_member_function_pointer_v<decltype(func)>, "RegistOneHandlerImpl: func is not a valid function");
        using ReturnType = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        if constexpr (kIsStreamGeneratorV<ReturnType>)
        {
            using StreamValueType = typename ReturnType::value_type;
            auto iter = stream_handler_map_.emplace(key, [&](std::string data, std::string attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> StreamGenerator<std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> StreamGenerator<std::string> {
                    return ExecuteStream_<serialize_protocol, func, StreamValueType>(std::move(data), std::move(attachment), nullptr);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: stream handler key already exists");
                return;
            }
            RpcLogInfo("[CoroRpcServer] Registered stream handler for key: %u", key);
        }
        else if constexpr(ToolBox::is_specialization_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string data, std::string attachment, typename rpc_protocol::supported_serialize_protocols protocols, request_context_ptr request_ctx) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                    return ExecuteCoro_<serialize_protocol, func>(std::move(data), std::move(attachment), request_ctx);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    return Execute_<serialize_protocol, func>(data, attachment);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        }
    }

    handler_t GetHandler(const rpc_func_key &key)
    {
        auto iter = rpc_server_handler_map_.find(key);
        if(iter == rpc_server_handler_map_.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    core_handler_t GetCoreHandler(const rpc_func_key &key)
    {
        auto iter = rpc_server_core_handler_map_.find(key);
        if(iter == rpc_server_core_handler_map_.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    ToolBox::coro::Task<std::pair<Errc, std::string>> HandleCoro(auto handler, std::string data, std::string attachment, typename rpc_protocol::supported_serialize_protocols protocols, const typename CoroRpcTools::rpc_func_key_t &key, request_context_ptr request_ctx)
    {
        using namespace std::string_literals;
        (void)key;
        if(handler) [[likely]]
        {
            try {
                auto pending = [&]() {
                    RequestContextBinder_ request_ctx_guard(request_ctx);
                    return handler(std::move(data), std::move(attachment), protocols, request_ctx);
                }();
                co_return co_await std::move(pending);
            } catch(Errc &err)
            {
                RpcLogError("[CoroRpcServer] HandleCoro: handler error, err:%d", err);
                co_return std::make_pair(err, ""); 
            } catch (const std::exception &e) {
                RpcLogError("[CoroRpcServer] HandleCoro: exception: %s", e.what());
                co_return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            } catch(...)
            {
                RpcLogError("[CoroRpcServer] HandleCoro: unknown exception");
                co_return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            }
        }else {
            RpcLogError("[CoroRpcServer] HandleCoro: handler not found");
            co_return std::make_pair(Errc::ERR_FUNC_NOT_REGISTERED, "");
        }
    }

    std::pair<Errc, std::string> Handle(auto handler, std::string_view payload, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols)
    {
        using namespace std::string_literals;
        if(handler) [[likely]]
        {
            try {
                return handler(payload, attachment, protocols);
            } catch(Errc &err)
            {
                RpcLogError("[CoroRpcServer] Handle: handler error, err:%d", err);
                return std::make_pair(err, ""); 
            } catch (const std::exception &e) {
                RpcLogError("[CoroRpcServer] Handle: exception: %s", e.what());
                return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            } catch(...)
            {
                RpcLogError("[CoroRpcServer] Handle: unknown exception");
                return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "")    ;
            }
        }else {
            RpcLogError("[CoroRpcServer] Handle: handler not found");
            return std::make_pair(Errc::ERR_FUNC_NOT_REGISTERED, "");
        }
    }

    // 统一的函数调用辅助：处理 Self 和参数的组合
    // 有参数版本
    template<auto func, typename Self, typename ArgsTuple>
    requires (!std::is_void_v<ArgsTuple>)
    static auto invoke_with_params(Self* self, ArgsTuple&& args) {
        if constexpr (std::is_void_v<Self>) {
            return std::apply(func, std::forward<ArgsTuple>(args));
        } else {
            return std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::forward<ArgsTuple>(args)));
        }
    }
    
    // 无参数版本
    template<auto func, typename Self>
    static auto invoke_with_params(Self* self) {
        if constexpr (std::is_void_v<Self>) {
            return func();
        } else {
            return std::apply(func, std::forward_as_tuple(*self));
        }
    }
    
    // 统一的函数调用辅助（协程版本）- 有参数版本
    template<auto func, typename Self, typename ArgsTuple>
    requires (!std::is_void_v<ArgsTuple>)
    static auto invoke_with_params_coro(Self* self, ArgsTuple&& args) {
        if constexpr (std::is_void_v<Self>) {
            return std::apply(func, std::forward<ArgsTuple>(args));
        } else {
            return std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::forward<ArgsTuple>(args)));
        }
    }
    
    // 无参数版本
    template<auto func, typename Self>
    static auto invoke_with_params_coro(Self* self) {
        if constexpr (std::is_void_v<Self>) {
            return func();
        } else {
            return std::apply(func, std::forward_as_tuple(*self));
        }
    }
    
    // 统一的执行逻辑：反序列化 -> 调用函数 -> 序列化返回值
    template<typename serialize_proto, auto func, typename Self = void>
    inline std::pair<Errc, std::string> Execute_(std::string_view data, std::string_view attachment, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using ReturnType = typename traits_type::return_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        (void)attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] Execute_: deserialize arguments failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] Execute_: Protocol compatibility check failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                invoke_with_params<func>(self, std::move(args));
                return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = invoke_with_params<func>(self, std::move(args));
                return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        } else {
            // 无参数函数
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] Execute_: Protocol compatibility check failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                invoke_with_params<func>(self);
                return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = invoke_with_params<func>(self);
                return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        }
    }

    template<typename serialize_proto, auto func, typename Self = void>
    inline coro::Task<std::pair<Errc, std::string>> ExecuteCoro_(std::string data, std::string attachment, request_context_ptr request_ctx, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using ReturnType = typename traits_type::return_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[rpc] Deserialize failed. data size:%zu, attachment size:%zu", data.size(), attachment.size());
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] ExecuteCoro_: Protocol compatibility check failed");
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                auto pending = [&]() {
                    RequestContextBinder_ request_ctx_guard(request_ctx);
                    return invoke_with_params_coro<func>(self, std::move(args));
                }();
                co_await std::move(pending);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto pending = [&]() {
                    RequestContextBinder_ request_ctx_guard(request_ctx);
                    return invoke_with_params_coro<func>(self, std::move(args));
                }();
                auto result = co_await std::move(pending);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        } else {
            // 无参数函数
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] ExecuteCoro_: Protocol compatibility check failed");
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                auto pending = [&]() {
                    RequestContextBinder_ request_ctx_guard(request_ctx);
                    return invoke_with_params_coro<func>(self);
                }();
                co_await std::move(pending);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto pending = [&]() {
                    RequestContextBinder_ request_ctx_guard(request_ctx);
                    return invoke_with_params_coro<func>(self);
                }();
                auto result = co_await std::move(pending);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        }
    }   // ExecuteCoro_
    
    // 执行流式RPC（成员函数版本）
    template<typename serialize_proto, auto func, typename StreamValueType, typename Self>
    requires (!std::is_void_v<Self>)
    inline StreamGenerator<std::string> ExecuteStream_(std::string data, std::string attachment, Self *self)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        (void)attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] ExecuteStream_: deserialize arguments failed");
                co_yield "";  // 返回错误标记
                co_return;
            }
            
            // 2. 调用函数获取StreamGenerator
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)));
            
            // 3. 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                // 如果 StreamValueType 是 std::string，直接使用字符串内容，不需要再次序列化
                if constexpr (std::is_same_v<StreamValueType, std::string>) {
                    co_yield value;
                } else {
                    std::string serialized = adapter::serialize(value);
                    co_yield std::move(serialized);
                }
            }
        } else {
            // 无参数函数
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::forward_as_tuple(*self));
            
            // 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                // 如果 StreamValueType 是 std::string，直接使用字符串内容，不需要再次序列化
                if constexpr (std::is_same_v<StreamValueType, std::string>) {
                    co_yield value;
                } else {
                    std::string serialized = adapter::serialize(value);
                    co_yield std::move(serialized);
                }
            }
        }
    }
    
    // 执行流式RPC（非成员函数版本）
    template<typename serialize_proto, auto func, typename StreamValueType>
    inline StreamGenerator<std::string> ExecuteStream_(std::string data, std::string attachment, std::nullptr_t)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] ExecuteStream_: deserialize arguments failed");
                co_yield "";  // 返回错误标记
                co_return;
            }
            
            // 2. 调用函数获取StreamGenerator
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::move(args));
            
            // 3. 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                // 如果 StreamValueType 是 std::string，直接使用字符串内容，不需要再次序列化
                if constexpr (std::is_same_v<StreamValueType, std::string>) {
                    co_yield value;
                } else {
                    std::string serialized = adapter::serialize(value);
                    co_yield std::move(serialized);
                }
            }
        } else {
            // 无参数函数
            StreamGenerator<StreamValueType> stream_gen = func();
            
            // 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                // 如果 StreamValueType 是 std::string，直接使用字符串内容，不需要再次序列化
                if constexpr (std::is_same_v<StreamValueType, std::string>) {
                    co_yield value;
                } else {
                    std::string serialized = adapter::serialize(value);
                    co_yield std::move(serialized);
                }
            }
        }
    }
    
    // 处理流式请求：注册活跃流并启动事件驱动发送
    void HandleStreamRequest_(uint64_t opaque, rpc_func_key key, const typename rpc_protocol::ReqHeader& req_header, StreamGenerator<std::string>&& generator, request_context_ptr request_ctx)
    {
        uint32_t stream_id = req_header.seq_num;

        // [DEFENSIVE] 防护性检测：如果 stream_id 已经存在于 active_streams_ 中，
        // 说明可能有多个客户端共享同一个 server 实例且 seq_num 冲突。
        // 当前架构下不应发生，如果触发此日志，说明需要升级为 (client_id, seq_num) 复合键。
        {
            std::lock_guard<mutex_type> lock(stream_state_mutex_);
            if (active_streams_.count(stream_id) > 0) {
                RpcLogError("[CoroRpcServer] HandleStreamRequest_: STREAM ID CONFLICT! stream_id: %u, client_id: %lu. "
                            "This indicates multiple clients sharing a single server instance with overlapping seq_num. "
                            "Consider upgrading to composite key (client_id, seq_num).",
                            stream_id, static_cast<unsigned long>(req_header.client_id));
            }
        }

        active_stream_ptr stream = std::make_shared<ActiveStream_>(ActiveStream_{
            std::move(generator), std::move(request_ctx),
            opaque, key, req_header,
            false, false, false, false, false
        });
        
        {
            std::lock_guard<mutex_type> lock(stream_state_mutex_);
            active_streams_[stream_id] = stream;
        }

        active_stream_ptr stream_to_pump;
        if (TryStartPump_(stream_id, stream_to_pump)) {
            SchedulePumpStream_(stream_id, std::move(stream_to_pump));
        }
    }
    
    // 发送流式响应
    Errc SendStreamResponse_(uint64_t opaque, rpc_func_key key, const typename rpc_protocol::ReqHeader& req_header, std::string&& data, uint8_t msg_type, Errc resp_err = Errc::SUCCESS)
    {
        (void)key;
        std::string resp_buf;
        resp_buf.resize(rpc_protocol::RESP_HEAD_LEN + data.size());
        
        bool prepare_ret = rpc_protocol::PrepareResponseHeader(resp_buf, data, req_header, 0, msg_type, resp_err);
        if (!prepare_ret) {
            RpcLogError("[CoroRpcServer] SendStreamResponse_: prepare response header failed");
            return Errc::ERR_SERVER_PREPARE_HEADER_FAILED;
        }
        
        std::memcpy(resp_buf.data() + rpc_protocol::RESP_HEAD_LEN, data.data(), data.size());
        
        if (!send_callback_) {
            RpcLogError("[CoroRpcServer] SendStreamResponse_: send callback not set");
            return Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        
        send_callback_(opaque, std::move(resp_buf));
        return Errc::SUCCESS;
    }
    
    Errc OnRecvReq_(uint64_t opaque, std::string_view data)
    {
        // 主动清理已完成的 detached coro tasks（惰性 GC）
        SweepDetachedTasks_();
        typename rpc_protocol::ReqHeader header;
        auto err = rpc_protocol::ReadHeader(data, header);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read header failed, err:%d", err);
            return err;
        }

        auto serialize_protocol = rpc_protocol::GetSerializeProtocol(header);
        if(!serialize_protocol.has_value())
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: serialize protocol not supported");
            return Errc::ERR_PROTOCOL;
        }

        std::string_view payload;
        std::string_view attachment;
        err = rpc_protocol::ReadPayLoad(header, data.substr(rpc_protocol::REQ_HEAD_LEN), payload, attachment);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read payload failed, err: %d", err);
            return err;
        }

        Errc resp_err = Errc::SUCCESS;
        std::string rpc_result;
        auto key = rpc_protocol::GetRpcFuncKey(header);
        uint32_t stream_id = header.seq_num;
        
        // 检查是否是流式RPC控制消息（取消、暂停、恢复）
        if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamCancel)) {
            // 处理流取消请求：若当前无人 pump，直接摘表；否则只打标记，由正在运行的 pump 清理
            {
                std::lock_guard<mutex_type> lock(stream_state_mutex_);
                auto iter = active_streams_.find(stream_id);
                if (iter != active_streams_.end() && iter->second) {
                    auto& stream = iter->second;
                    stream->cancelled = true;
                    RpcLogInfo("[CoroRpcServer] OnRecvReq: stream cancelled by client, stream_id: %u", stream_id);
                    if (!stream->pumping) {
                        stream->finished = true;
                        active_streams_.erase(iter);
                    }
                }
            }
            return Errc::SUCCESS;
        } else if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamPause)) {
            // 处理流暂停请求（背压控制）—— 只改状态，不阻塞线程
            std::lock_guard<mutex_type> lock(stream_state_mutex_);
            auto iter = active_streams_.find(stream_id);
            if (iter != active_streams_.end() && iter->second) {
                iter->second->paused = true;
                RpcLogInfo("[CoroRpcServer] OnRecvReq: stream paused (backpressure), stream_id: %u", stream_id);
            }
            return Errc::SUCCESS;
        } else if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamResume)) {
            // 处理流恢复请求（背压控制）—— 取消暂停并在锁外继续推进
            active_stream_ptr stream_to_pump;
            {
                std::lock_guard<mutex_type> lock(stream_state_mutex_);
                auto iter = active_streams_.find(stream_id);
                if (iter != active_streams_.end() && iter->second) {
                    iter->second->paused = false;
                    RpcLogInfo("[CoroRpcServer] OnRecvReq: stream resumed (backpressure), stream_id: %u", stream_id);
                    if (!iter->second->pumping && !iter->second->cancelled && !iter->second->finished) {
                        iter->second->pumping = true;
                        stream_to_pump = iter->second;
                    }
                }
            }
            if (stream_to_pump) {
                SchedulePumpStream_(stream_id, std::move(stream_to_pump));
            }
            return Errc::SUCCESS;
        }
        
        auto request_ctx = CreateRequestContext_(attachment);

        // 检查是否是流式RPC请求
        bool is_stream_request = (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart));
        
        if (is_stream_request) {
            // 处理流式RPC请求
            auto stream_iter = stream_handler_map_.find(key);
            if (stream_iter != stream_handler_map_.end()) {
                // 执行流式RPC，获取StreamGenerator
                RequestContextBinder_ request_ctx_guard(request_ctx);
                auto stream_gen = stream_iter->second(std::string(payload), std::string(attachment), serialize_protocol.value());
                
                // 启动流式响应处理（在后台协程中）
                HandleStreamRequest_(opaque, key, header, std::move(stream_gen), request_ctx);
                return Errc::SUCCESS;
            } else {
                RpcLogError("[CoroRpcServer] OnRecvReq: stream handler not found for key: %u", key);
                // 回错误包而非仅返回错误码
                SendStreamResponse_(opaque, key, header, "", static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamError), Errc::ERR_FUNC_NOT_REGISTERED);
                return Errc::SUCCESS;
            }
        }
        
        // 普通RPC处理
        auto handler = GetHandler(key);
        if(handler == nullptr)
        {
            auto core_handler = GetCoreHandler(key);
            if (!core_handler) {
                // 函数未注册 → 回错误包而不是仅返回错误码
                RpcLogError("[CoroRpcServer] OnRecvReq: handler not found for key: %u", key);
                ResponseMsgAttachment_(opaque, key, Errc::ERR_FUNC_NOT_REGISTERED, std::string{}, header);
                return Errc::SUCCESS;
            }
            auto handler_ret = HandleCoro(core_handler, std::string(payload), std::string(attachment), serialize_protocol.value(), key, request_ctx);
            DetachCoroTask_(std::move(handler_ret), [this, opaque, key, header, request_ctx](std::pair<Errc, std::string> result) mutable {
                RequestContextBinder_ request_ctx_guard(request_ctx);
                // 无论成功还是失败，都回包（不再吞错只打日志）
                if(result.first != Errc::SUCCESS)
                {
                    RpcLogError("[CoroRpcServer] OnRecvReq: coro handler error, err:%d", result.first);
                }
                ResponseMsgAttachment_(opaque, key, result.first, result.second, header);
            });
            return Errc::SUCCESS;
        }else {
            RequestContextBinder_ request_ctx_guard(request_ctx);
            std::tie(resp_err, rpc_result) = Handle(handler, payload, attachment, serialize_protocol.value());
            if(resp_err != Errc::SUCCESS)
            {
                RpcLogError("[CoroRpcServer] OnRecvReq: handler error, err:%d", resp_err);
                // 出错时也回错误响应包，而不是仅返回错误码
            }
            return ResponseMsgAttachment_(opaque, key, resp_err, rpc_result, header);
        }
    }
    Errc ResponseMsgAttachment_(uint64_t opaque, CoroRpcTools::rpc_func_key key, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_header)
    {
        Errc ret = Errc::SUCCESS;
        // 优先使用 per-request context 中 handler 设置的 resp_attachment
        if (current_request_ctx_ && current_request_ctx_->has_resp_attachment) {
            std::string owned_attachment = std::move(current_request_ctx_->resp_attachment);
            current_request_ctx_->resp_attachment.clear();
            current_request_ctx_->has_resp_attachment = false;
            ret = DirectResponseMsg(opaque, resp_err, rpc_result, req_header, 
                [&owned_attachment](){ return std::string_view{owned_attachment}; });
        } else {
            // 降级：使用函数级全局 SetRespAttachmentFunc 注册的回调
            // 注意：不能 move 出 map 中的回调，否则后续请求会触发 bad_function_call
            auto resp_attachment_func_iter = resp_attachment_func_map_.find(key);
            if(resp_attachment_func_iter != resp_attachment_func_map_.end())
            {
                ret = DirectResponseMsg(opaque, resp_err, rpc_result, req_header, resp_attachment_func_iter->second);
            } else {
                ret = DirectResponseMsg(opaque, resp_err, rpc_result, req_header);
            }
        }
        if(ret != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: prepare response header failed");
            return ret;
        }
        return Errc::SUCCESS;
    }
    Errc DirectResponseMsg(uint64_t opaque, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head) {
        return DirectResponseMsg(opaque, resp_err, std::move(rpc_result), req_head, 
                               [](){ return std::string_view{}; });
    }
    Errc DirectResponseMsg(uint64_t opaque, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head,
        std::function<std::string_view()> attachment_func)
    {
        std::string resp_error_msg;
        if(resp_err != Errc::SUCCESS)
        {
            resp_error_msg = rpc_result;
            RpcLogWarn("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", resp_err);
        }
        std::string_view attachment = attachment_func();
        std::string resp_buf_bytes;
        resp_buf_bytes.resize(rpc_protocol::RESP_HEAD_LEN + rpc_result.size() + attachment.length());
        // RpcLogDebug("[rpc][server] DirectResPonseMsg.resp_buf_bytes: resp_buf_bytes size: %zu, rpc_result: %zu, attachment_size: %zu", resp_buf_bytes.size(), rpc_result.size(), attachment.length());
        bool prepare_ret = rpc_protocol::PrepareResponseHeader(resp_buf_bytes, rpc_result, req_head, attachment.length(), resp_err, resp_error_msg);
        if(!prepare_ret)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: prepare response header failed");
            return Errc::ERR_SERVER_PREPARE_HEADER_FAILED;
        }
        auto response_ret = Response(opaque, std::move(resp_buf_bytes), rpc_result, std::move(attachment));
        Errc err_code = response_ret.get_result();
        if(err_code != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", err_code);
            return err_code;
        }
        return Errc::SUCCESS;
    }
    ToolBox::coro::Task<Errc> Response(uint64_t opaque, std::string &&resp_buf_bytes, const std::string &rpc_result, std::string_view &&attachment)
    {
        if(!send_callback_)
        {
            RpcLogError("[CoroRpcServer] Response: send callback not set");
            co_return Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN, rpc_result.data(), rpc_result.size());
        if(attachment.length() > 0)
        {
            std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN + rpc_result.size(), attachment.data(), attachment.size());
        }
        send_callback_(opaque, std::move(resp_buf_bytes));
        co_return Errc::SUCCESS;
    }
};  // class CoroRpcServer

}   // namespace ToolBox::CoroRpc
}   // namespace ToolBox