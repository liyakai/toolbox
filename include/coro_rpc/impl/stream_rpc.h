#pragma once

#include <coroutine>
#include <mutex>
#include <optional>
#include <queue>
#include <functional>

#include "coro_rpc_def_interenal.h"

namespace ToolBox::CoroRpc {

// 流式RPC生成器 - 用于服务器端生成流式数据
// T: 流式数据的类型
template<typename T>
class StreamGenerator {
public:
    using value_type = T;
    
    struct promise_type {
        T current_value;
        std::exception_ptr exception;
        std::coroutine_handle<> continuation;
        
        StreamGenerator get_return_object() {
            return StreamGenerator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        void return_void() {}
    };
    
    using handle_type = std::coroutine_handle<promise_type>;
    
    StreamGenerator(handle_type h) : coro_(h) {}
    
    ~StreamGenerator() {
        if (coro_) {
            coro_.destroy();
        }
    }
    
    // 禁止拷贝，允许移动
    StreamGenerator(const StreamGenerator&) = delete;
    StreamGenerator& operator=(const StreamGenerator&) = delete;
    StreamGenerator(StreamGenerator&& other) noexcept : coro_(other.coro_) {
        other.coro_ = {};
    }
    StreamGenerator& operator=(StreamGenerator&& other) noexcept {
        if (this != &other) {
            if (coro_) {
                coro_.destroy();
            }
            coro_ = other.coro_;
            other.coro_ = {};
        }
        return *this;
    }
    
    // 获取下一个值（如果可用）
    // 返回: 如果有下一个值返回true，否则返回false（流结束）
    bool Next() {
        if (!coro_ || coro_.done()) {
            return false;
        }

        coro_.resume();

        if (coro_.promise().exception) {
            std::rethrow_exception(coro_.promise().exception);
        }

        return !coro_.done();
    }
    
    // 获取当前值
    T& value() {
        return coro_.promise().current_value;
    }
    
    // 检查流是否已结束
    bool done() const {
        return !coro_ || coro_.done();
    }
    
    // 手动销毁协程
    void Destroy() {
        if (coro_) {
            coro_.destroy();
            coro_ = {};
        }
    }
    
private:
    handle_type coro_;
};

// 流式RPC读取器 - 用于客户端接收流式数据
// T: 流式数据的类型
// MutexType: 保护本地缓冲区状态的锁类型，默认无锁 NullMutex

template<typename T, typename MutexType = NullMutex>
class StreamReader {
public:
    using value_type = T;
    using mutex_type = MutexType;
    using BackpressureCallback = std::function<void(uint32_t stream_id, bool pause)>;  // stream_id, pause/resume
    
    struct StreamAwaiter {
        StreamReader* reader;

        // 只要已有缓存值，或者流已经进入终态，就不需要再挂起。
        // 终态下 await_resume() 会返回 std::nullopt。
        bool await_ready() const noexcept {
            return reader->HasValue() || reader->IsFinished();
        }
        
        // 返回 bool：true = 成功挂起（等待 PushValue/Finish 唤醒）；
        //            false = 已有数据/已终止，运行时立刻恢复协程。
        // 这样保证只有一条路径负责恢复协程，彻底消除 double-resume UB。
        bool await_suspend(std::coroutine_handle<> h) {
            return reader->TrySetContinuation(h);
        }
        
        std::optional<T> await_resume() {
            return reader->PopValue();
        }
    };
    
    // 构造函数：可以指定缓冲区大小限制（默认1000）
    explicit StreamReader(size_t max_buffer_size = 1000) 
        : max_buffer_size_(max_buffer_size) {}
    ~StreamReader() = default;
    
    // 禁止拷贝，允许移动
    StreamReader(const StreamReader&) = delete;
    StreamReader& operator=(const StreamReader&) = delete;
    StreamReader(StreamReader&&) = default;
    StreamReader& operator=(StreamReader&&) = default;
    
    // 推送一个值到流中
    // 返回: true 表示未超过软上限；false 表示当前值已保存，但缓冲区已满/超限，需要背压控制。
    bool PushValue(T value) {
        std::coroutine_handle<> h_to_resume;
        BackpressureCallback backpressure_callback;
        uint32_t stream_id = 0;
        bool should_pause = false;
        bool buffer_was_full = false;
        {
            std::lock_guard<mutex_type> lock(mutex_);

            buffer_was_full = !finished_ && values_.size() >= max_buffer_size_;
            values_.push(std::move(value));

            // 软上限：当前 chunk 仍然保留，背压用于尽快阻止后续继续灌入，避免永久丢包。
            if (!finished_ && values_.size() >= max_buffer_size_ && !paused_) {
                paused_ = true;
                backpressure_callback = backpressure_callback_;
                stream_id = stream_id_;
                should_pause = true;
            }

            // 保存 continuation 并在释放锁后 resume，避免死锁
            if (continuation_) {
                h_to_resume = continuation_;
                continuation_ = {};
            }
        }
        if (should_pause && backpressure_callback) {
            backpressure_callback(stream_id, true);
        }
        // 在锁释放后 resume 协程，避免死锁
        if (h_to_resume) {
            h_to_resume.resume();
        }
        return !buffer_was_full;
    }
    
    // 标记流正常结束。
    // 调用后表示不会再有新值到达，但已缓冲的值仍可继续通过 PopValue() 读取。
    void Finish() {
        std::coroutine_handle<> h_to_resume;
        {
            std::lock_guard<mutex_type> lock(mutex_);
            finished_ = true;
            if (continuation_) {
                h_to_resume = continuation_;
                continuation_ = {};
            }
        }
        // 在锁释放后 resume 协程，避免死锁
        if (h_to_resume) {
            h_to_resume.resume();
        }
    }
    
    // 标记流异常结束。
    // 调用后流进入终态，GetError() 可读取错误；PopValue() 不再返回数据。
    void SetError(Errc err) {
        std::coroutine_handle<> h_to_resume;
        {
            std::lock_guard<mutex_type> lock(mutex_);
            error_ = err;
            finished_ = true;
            if (continuation_) {
                h_to_resume = continuation_;
                continuation_ = {};
            }
        }
        // 在锁释放后 resume 协程，避免死锁
        if (h_to_resume) {
            h_to_resume.resume();
        }
    }
    
    // 检查本地缓冲区中是否还有已到达但尚未消费的值。
    // 该状态与 IsFinished() 独立：流已经结束时，这里仍可能返回 true。
    bool HasValue() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return !values_.empty();
    }
    
    // 弹出下一个已缓冲的值。
    // 返回 std::nullopt 表示当前没有可读值；如果流已结束且缓冲为空，也会返回 std::nullopt。
    std::optional<T> PopValue() {
        BackpressureCallback backpressure_callback;
        uint32_t stream_id = 0;
        bool should_resume = false;
        std::optional<T> value;
        {
            std::lock_guard<mutex_type> lock(mutex_);
            if (error_.has_value()) {
                return std::nullopt;
            }
            if (values_.empty()) {
                return std::nullopt;
            }
            value = std::move(values_.front());
            values_.pop();

            // 如果之前是暂停状态，现在有空间了，可以恢复
            if (paused_ && values_.size() < max_buffer_size_ / 2) {
                paused_ = false;
                backpressure_callback = backpressure_callback_;
                stream_id = stream_id_;
                should_resume = true;
            }
        }
        if (should_resume && backpressure_callback) {
            backpressure_callback(stream_id, false);
        }
        return value;
    }
    
    // 尝试注册协程继续点（供 StreamAwaiter::await_suspend 使用）。
    // 返回 true  = 成功挂起，等待后续 PushValue/Finish/SetError 唤醒
    // 返回 false = 已经有值或已终态，运行时会立刻恢复协程
    // 这保证了只有一条路径负责恢复协程，彻底消除 double-resume UB。
    bool TrySetContinuation(std::coroutine_handle<> h) {
        std::lock_guard<mutex_type> lock(mutex_);
        if (!values_.empty() || finished_) {
            // 已有数据或已结束 → 不挂起
            return false;
        }
        continuation_ = h;
        return true;
    }
    
    // 设置协程继续点（外部手动调用版本，兼容旧用法）。
    // 只要已经有值可读，或流已进入终态，就会立即恢复等待方。
    void SetContinuation(std::coroutine_handle<> h) {
        bool should_resume = false;
        {
            std::lock_guard<mutex_type> lock(mutex_);
            continuation_ = h;
            if (!values_.empty() || finished_) {
                continuation_ = {};
                should_resume = true;
            }
        }
        // 在锁释放后 resume 协程，避免死锁
        if (should_resume) {
            h.resume();
        }
    }
    
    // 检查流是否已进入终态。
    // 返回 true 只表示不会再有新数据到达，不表示本地缓冲区已经读空。
    // 因此读取端应优先用 while (!reader.IsFinished() || reader.HasValue()) 来消费全部数据。
    bool IsFinished() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return finished_;
    }
    
    // 获取错误码（如果有）
    std::optional<Errc> GetError() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return error_;
    }
    
    /**
     * @brief 创建awaiter用于co_await
     */
    StreamAwaiter operator co_await() {
        return StreamAwaiter{this};
    }
    
    // 设置流ID（用于背压回调）
    void SetStreamId(uint32_t stream_id) {
        std::lock_guard<mutex_type> lock(mutex_);
        stream_id_ = stream_id;
    }
    
    // 设置背压回调函数
    void SetBackpressureCallback(BackpressureCallback callback) {
        std::lock_guard<mutex_type> lock(mutex_);
        backpressure_callback_ = std::move(callback);
    }
    
    // 取消流（客户端主动取消）。
    // 调用后流进入终态，不会再接收新数据；实际取消消息仍应由客户端通过 CancelStream 发送。
    void Cancel() {
        std::coroutine_handle<> h_to_resume;
        {
            std::lock_guard<mutex_type> lock(mutex_);
            if (!finished_ && !cancelled_) {
                cancelled_ = true;
                finished_ = true;
                if (continuation_) {
                    h_to_resume = continuation_;
                    continuation_ = {};
                }
            }
        }
        // 在锁释放后 resume 协程，避免死锁
        if (h_to_resume) {
            h_to_resume.resume();
        }
    }
    
    // 检查是否已取消
    bool IsCancelled() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return cancelled_;
    }
    
    // 获取当前缓冲区大小
    size_t GetBufferSize() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return values_.size();
    }
    
    // 获取最大缓冲区大小
    size_t GetMaxBufferSize() const {
        return max_buffer_size_;
    }
    
    // 检查是否处于暂停状态（背压）
    bool IsPaused() const {
        std::lock_guard<mutex_type> lock(mutex_);
        return paused_;
    }
    
private:
    mutable mutex_type mutex_;
    std::queue<T> values_;
    std::coroutine_handle<> continuation_;
    bool finished_ = false;
    bool cancelled_ = false;
    bool paused_ = false;  // 背压暂停状态
    std::optional<Errc> error_;
    size_t max_buffer_size_;  // 最大缓冲区大小
    uint32_t stream_id_ = 0;  // 流ID（用于背压回调）
    BackpressureCallback backpressure_callback_;  // 背压回调函数
};

} // namespace ToolBox::CoroRpc
