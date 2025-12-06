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
template<typename T>
class StreamReader {
public:
    using value_type = T;
    using BackpressureCallback = std::function<void(uint32_t stream_id, bool pause)>;  // stream_id, pause/resume
    
    struct StreamAwaiter {
        StreamReader* reader;
        bool await_ready() const noexcept {
            return reader->HasValue();
        }
        
        void await_suspend(std::coroutine_handle<> h) {
            reader->SetContinuation(h);
            if (reader->HasValue()) {
                h.resume();
            }
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
    // 返回: true表示成功，false表示缓冲区已满（需要背压控制）
    bool PushValue(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查缓冲区是否已满
        if (values_.size() >= max_buffer_size_ && !finished_) {
            // 缓冲区满，需要触发背压
            if (!paused_) {
                paused_ = true;
                if (backpressure_callback_) {
                    // 通知服务器暂停发送
                    backpressure_callback_(stream_id_, true);
                }
            }
            return false;  // 缓冲区满，无法接收
        }
        
        values_.push(std::move(value));
        
        // 如果之前是暂停状态，现在有空间了，可以恢复
        if (paused_ && values_.size() < max_buffer_size_ / 2) {
            paused_ = false;
            if (backpressure_callback_) {
                // 通知服务器恢复发送
                backpressure_callback_(stream_id_, false);
            }
        }
        
        if (continuation_) {
            auto h = continuation_;
            continuation_ = {};
            h.resume();
        }
        return true;
    }
    
    // 标记流结束
    void Finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        if (continuation_) {
            auto h = continuation_;
            continuation_ = {};
            h.resume();
        }
    }
    
    // 标记流错误
    void SetError(Errc err) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = err;
        finished_ = true;
        if (continuation_) {
            auto h = continuation_;
            continuation_ = {};
            h.resume();
        }
    }
    
    // 检查是否有值可用
    bool HasValue() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !values_.empty();
    }
    
    // 弹出下一个值
    std::optional<T> PopValue() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (error_.has_value()) {
            return std::nullopt;
        }
        if (values_.empty()) {
            if (finished_) {
                return std::nullopt;
            }
            return std::nullopt;
        }
        T value = std::move(values_.front());
        values_.pop();
        
        // 如果之前是暂停状态，现在有空间了，可以恢复
        if (paused_ && values_.size() < max_buffer_size_ / 2) {
            paused_ = false;
            if (backpressure_callback_) {
                // 通知服务器恢复发送
                backpressure_callback_(stream_id_, false);
            }
        }
        
        return value;
    }
    
    // 设置协程继续点
    void SetContinuation(std::coroutine_handle<> h) {
        std::lock_guard<std::mutex> lock(mutex_);
        continuation_ = h;
        if (!values_.empty() || finished_) {
            continuation_ = {};
            h.resume();
        }
    }
    
    // 检查流是否已结束
    bool IsFinished() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return finished_ && values_.empty();
    }
    
    // 获取错误码（如果有）
    std::optional<Errc> GetError() const {
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
        stream_id_ = stream_id;
    }
    
    // 设置背压回调函数
    void SetBackpressureCallback(BackpressureCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        backpressure_callback_ = std::move(callback);
    }
    
    // 取消流（客户端主动取消）
    // 注意：这个方法只标记流为取消状态，实际的取消消息应该由客户端通过CancelStream发送
    void Cancel() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!finished_ && !cancelled_) {
            cancelled_ = true;
            finished_ = true;
            if (continuation_) {
                auto h = continuation_;
                continuation_ = {};
                h.resume();
            }
        }
    }
    
    // 检查是否已取消
    bool IsCancelled() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cancelled_;
    }
    
    // 获取当前缓冲区大小
    size_t GetBufferSize() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return values_.size();
    }
    
    // 获取最大缓冲区大小
    size_t GetMaxBufferSize() const {
        return max_buffer_size_;
    }
    
    // 检查是否处于暂停状态（背压）
    bool IsPaused() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return paused_;
    }
    
private:
    mutable std::mutex mutex_;
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

