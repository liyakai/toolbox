#pragma once

#include <atomic>
#include <coroutine>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <utility>
#include <optional>
#include <list>
#include <condition_variable>
#include <thread>
#include <functional>
#include <future>

#include "tools/ringbuffer.h"


namespace ToolBox {

namespace coro {

class SharedLooperExecutor;

template <typename ResultType, typename Executor> struct Task;
template <typename ResultType, typename Executor> struct TaskPromise;

// 调度器接口
class IExecutor {
public:
  virtual ~IExecutor() = default;
  virtual void execute(std::function<void()> &&func) = 0;
};

template <typename T> 
struct Result 
{
    // 初始化为默认值
    explicit Result() = delete;

    // 当 Task 正常返回是用结果初始化 Result
    explicit Result(T &&value) : _value(std::move(value)), _has_value(true) {}

    // 当 Task 抛异常时用异常初始化 Result
    explicit Result(std::exception_ptr &&exception_ptr)
        : _value(std::nullopt), _exception_ptr(exception_ptr), _has_value(false) {}

    // 读取结果,有异常则抛出异常
    T get_or_throw() {
        if (!_has_value) {
            std::rethrow_exception(_exception_ptr);
        }
        return std::move(_value.value());
    }

private:
    std::optional<T> _value;
    std::exception_ptr _exception_ptr;
    bool _has_value = false;
};

struct DispatchAwaiter {
    explicit DispatchAwaiter(IExecutor &executor) noexcept
        : executor_(executor) {}

    bool await_ready() const { return false; }
    // coroutine_handle 类型的参数。这是一个由编译器生成的变量。在此函数中调用
    // handle.resume()，就可以恢复协程。
    void await_suspend(std::coroutine_handle<> handle) const {
        // 调度到协程对应的调度器上
        executor_.execute([handle]() { handle.resume(); });
    }

    void await_resume() {}

private:
    IExecutor &executor_;
};

template <typename Result, typename Executor> 
struct TaskAwaiter 
{
    explicit TaskAwaiter(IExecutor &executor,
                        Task<Result, Executor> &&task) noexcept
        : executor_(executor), task_(std::move(task)) {}
    TaskAwaiter(TaskAwaiter &&completion) noexcept
        : task_(std::exchange(completion.task, {})) {}
    TaskAwaiter(TaskAwaiter &) = delete;
    TaskAwaiter &operator=(TaskAwaiter &) = delete;
    // 必要的法定函数.
    constexpr bool await_ready() const noexcept { return false; }
    // 必要的法定函数. coroutine_handle
    // 类型的参数。这是一个由编译器生成的变量。在此函数中调用
    // handle.resume()，就可以恢复协程。
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        // 当 task 执行完之后调用 resume
        task_.finally([handle, this]() {
            // 将 resume 函数的调用交给调度器执行
            executor_.execute([handle]() { handle.resume(); });
        });
    }
    // 协程恢复执行时,被等待的Task 已经执行完,调用 get_result 来获取结果
    // await_resume 的返回值类型也是不限定的，返回值将作为 co_await 表达式的返回值
    Result await_resume() noexcept { return task_.get_result(); }

private:
    IExecutor &executor_;
    Task<Result, Executor> task_; // 二阶Task,即协程体内部的 co_await 获得的 task.
};

template <typename ResultType, typename Executor = SharedLooperExecutor>
struct Task {
    // 声明 promise_type 为 TaskPromise 类型
    using promise_type = TaskPromise<ResultType, Executor>;

    ResultType get_result() { return handle.promise().get_result(); }
    Task &then(std::function<void(ResultType)> &&func) {
        // 将回调函数 func 注册到 promise 中.
        handle.promise().on_completed([func](auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {
                // 忽略异常
            }
        });
        return *this;
    }
    Task &catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func](auto result) {
        try {
            // 忽略返回值
            result.get_or_throw();
        } catch (std::exception &e) {
            func(e);
        }
        });
        return *this;
    }

    Task &finally(std::function<void()> &&func) {
        handle.promise().on_completed([func](auto&) { func(); });
        return *this;
    }
    explicit Task(std::coroutine_handle<promise_type> handle) noexcept
        : handle(handle) {}
    Task(Task &&task) noexcept : handle(std::exchange(task.handle, {})) {}
    Task(Task &) = delete;
    Task &operator=(Task &) = delete;
    ~Task() {
        if (handle) {
            // 在销毁前添加协程句柄的空值检查
            auto h = std::exchange(handle, nullptr);
            if (h && h.done()) {
                h.destroy();
            }
        }
    }

private:
    std::coroutine_handle<promise_type> handle;
};

template <typename ResultType, typename Executor> 
struct TaskPromise {
    // 构造协程的返回值对象 Task [此函数是法定构建 Result 对象的函数]
    // 协程的状态被创建出来之后，会立即构造 promise_type 对象，进而调用
    // get_return_object 来创建返回值对象
    Task<ResultType, Executor> get_return_object() {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    // 法定函数. 协程立即执行
    // 协程体执行的第一步是调用 co_await
    // promise.initial_suspend()，initial_suspend
    // 的返回值就是一个等待对象（awaiter） 写成启动时也需要在恢复时实现调度
    DispatchAwaiter initial_suspend() { return DispatchAwaiter{executor_}; }

    // 法定函数. 执行结束后挂起,等待外部销毁,该逻辑与前面的 Generator 类似
    std::suspend_always final_suspend() noexcept { return {}; }

    // 法定函数.
    void unhandled_exception() {
        std::lock_guard lock(completion_lock);
        // 将异常存入 result
        result.emplace(Result<ResultType>(std::current_exception()));
        // 通知 get_result 当中的 wait
        completion.notify_all();
        // 调用回调
        notify_callbacks();
    }

    // 法定函数. co_return 10000; 1000 会作为参数传入, 即 return_value 函数的参数
    // value 的值为 1000 void 版本为 return_void
    void return_value(ResultType value) {
        // 限制锁的范围
        {
            std::lock_guard lock(completion_lock);
            // 将返回值存入 result, 对应于协程内部的 'co_return value'
            result.emplace(std::move(value));
            // 通知 get_result 当中的 wait
            completion.notify_all();
        }

        // 调用回调
        notify_callbacks();
    }
    // 自定义函数,获取结果,如果还没有结果就等待.
    ResultType get_result() {
        // 如果 result 没有值, 说明协程还没有运行完,等待值被写入再返回.
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            // 等待写入值之后调用 notify_all
            completion.wait(lock);
        }
        return result->get_or_throw();
    }

    // 定义了 await_transform 函数之后，co_await expr 就相当于 co_await
    // promise.await_transform(expr) 了
    template <typename _ResultType, typename _Executor>
    TaskAwaiter<_ResultType, _Executor>
    await_transform(Task<_ResultType, _Executor> &&task) {
        return TaskAwaiter<_ResultType, _Executor>(executor_, std::move(task));
    }



    // 添加一个通用的 await_transform 方法
    template <typename T>
    decltype(auto) await_transform(T&& awaitable) {
        return std::forward<T>(awaitable);
    }


    void on_completed(std::function<void(Result<ResultType>&)> &&func) {
        std::unique_lock lock(completion_lock);
        // 加锁判断 result
        if (result.has_value()) {
            // result 已经有值.
            auto &value = result.value();
            // 解锁之后再调用 func
            lock.unlock();
            func(std::ref(value));
        } else {
        // 否则添加回调函数,等待调用
            completion_callbacks.push_back(std::move(func));
        }
    }

private:
    // 使用 std::optional 可以区分协程是否执行完成.
    std::optional<Result<ResultType>> result;

    std::mutex completion_lock;
    std::condition_variable completion;

    // 回调列表,允许对同一个 Task 添加多个回调.
    std::list<std::function<void(Result<ResultType>&)>> completion_callbacks;

    void notify_callbacks() {
        auto &value = result.value();
        for (auto &callback : completion_callbacks) {
            callback(std::ref(value));
        }
        // 调用完成,清空回调
        completion_callbacks.clear();
    }
    Executor executor_;
};

// ----------------------------------------------------------------------------
// 调度器实现

class NoopExecutor : public IExecutor {
public:
  void execute(std::function<void()> &&func) override { func(); }
};

class NewThreadExecutor : public IExecutor {
public:
    void execute(std::function<void()> &&func) override {
        std::thread mythread(std::move(func));
        mythread.join();
    }
};

class AsyncExecutor : public IExecutor {
public:
    void execute(std::function<void()> &&func) override {
        auto future = std::async(func);
    }
};

class LooperExecutor : public IExecutor {
private:
    std::condition_variable queue_condition;
    std::mutex queue_lock;
    std::queue<std::function<void()>> executable_queue;

    std::atomic<bool> is_active; // true是工作状态,如果要关闭事件循环,就置为 false
    std::thread work_thread;

private:
    // 处理事件循环
    void run_loop() {
        // 检查当前事件循环是否是工作状态,或者队列没有清空.
        while (is_active.load(std::memory_order_relaxed) ||
            !executable_queue.empty()) {
        std::unique_lock lock(queue_lock);
        if (executable_queue.empty()) {
            // 队列为空,需要等待新任务加入队列或者关闭事件循环的通知.
            queue_condition.wait(lock);
            // 如果队列为空,那么说明收到的是关闭的通知
            if (executable_queue.empty()) {
                // 现有逻辑下此处用 break 也可
                // 使用 continue 可以再次检查状态和队列,方便将来扩展
                continue;
            }
        }

        // 取出第一个任务,解锁再执行.
        // func 是外部逻辑,不需要锁保护; func 当中可能请求锁,导致死锁.
        auto func = executable_queue.front();
        executable_queue.pop();
        lock.unlock();
        func();
        }
  }

public:
    LooperExecutor() {
        work_thread = std::thread(&LooperExecutor::run_loop, this);
        is_active.store(true, std::memory_order_relaxed);
    }
    ~LooperExecutor() {
        shutdown(false);
        // 等待线程执行完,防止出现意外情况.
        join();
    }

    void execute(std::function<void()> &&func) override {
        std::unique_lock lock(queue_lock);
        if (is_active.load(std::memory_order_relaxed)) {
            executable_queue.push(func);
            lock.unlock();
            // 通知队列,主要用于队列之前为空时调用 wait 等待的情况
            // 通知不需要加锁,否则锁会交给 wait 方导致当前线程阻塞
            queue_condition.notify_one();
        }
    }
    void shutdown(bool wait_for_complete = true) {
        // 修改后立即生效,在 run_loop 当中就能尽早(加锁前)就检测到 is_active 的变化
        is_active.store(false, std::memory_order_relaxed);
        if (!wait_for_complete) {
            std::unique_lock lock(queue_lock);
            // 清空任务队列
            decltype(executable_queue) empty_queue;
            executable_queue.swap(empty_queue);
            lock.unlock();
        }
        // 通知 wait 函数, 避免 Looper 线程不退出
        // 不需要加锁, 否则锁会交给 wait 放导致当前线程阻塞.
        queue_condition.notify_all();
    }

    void join() {
        if (work_thread.joinable()) {
            work_thread.join();
        }
    }
};

// RingBufferSPSCEventExecutor 实现方式略显暴力,但其响应速度比LooperExecutor更佳.
#define LOOPER_EVENT_QUEUE_MAX_COUNT 10240
class RingBufferSPSCEventExecutor : public IExecutor {
private:
    RingBufferSPSC<std::function<void()>, LOOPER_EVENT_QUEUE_MAX_COUNT> executable_queue;
    std::atomic<bool> is_active; // true是工作状态,如果要关闭事件循环,就置为 false
    std::thread work_thread;

private:
    // 处理事件循环
    void run_loop() {
        int idle_count = 0;
        // 检查当前事件循环是否是工作状态,或者队列没有清空.
        while (is_active.load(std::memory_order_relaxed)) {

            if (executable_queue.Empty()) 
            {
                if (idle_count < 100) 
                {
                    std::this_thread::yield();
                }
                else 
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
                idle_count++;
                continue;
            }
            while (!executable_queue.Empty()) 
            {
                auto func = executable_queue.Pop();
                func();
            }
            std::this_thread::yield(); // 处理一批后让出CPU
            idle_count = 0;
        }
    }

public:
    RingBufferSPSCEventExecutor() {
        work_thread = std::thread(&RingBufferSPSCEventExecutor::run_loop, this);
        is_active.store(true, std::memory_order_relaxed);
    }
    ~RingBufferSPSCEventExecutor() {
        shutdown(false);
        // 等待线程执行完,防止出现意外情况.
        join();
    }

    void execute(std::function<void()> &&func) override {
        if (is_active.load(std::memory_order_relaxed)) {
            if (executable_queue.Full()) 
            {
                // TO DO: add error log
                fprintf(stderr,"[coroutine] execute failed for executable_queue is full.");
            }
            executable_queue.Push(std::move(func));
        }
    }
    void shutdown(bool wait_for_complete = true) {
        is_active.store(false, std::memory_order_relaxed);
        if (!wait_for_complete) {
            // 清空任务队列
            executable_queue.Clear();
        }
    }

    void join() {
        if (work_thread.joinable()) {
            work_thread.join();
        }
    }
};

// RingBufferSPSC<NetEventWorker*, NETWORK_EVENT_QUEUE_MAX_COUNT>;

class SharedLooperExecutor : public IExecutor {
public:
    void execute(std::function<void()> &&func) override {
        //static LooperExecutor shared_looper;
        // static RingBufferSPSCEventExecutor shared_looper;
        static NoopExecutor shared_looper;
        shared_looper.execute(std::move(func));
    }
};

// ----------------------------------------------------------------------------
// 将回调式的异步操作转换为写成可等待的形式

template <typename Arg, typename Derived>
class CallbackAwaitroBase{
private:
    template <typename Op>
    class CallbackAwaitorImpl {
        public:
            CallbackAwaitorImpl(Derived &awaitor, Op &op):awaitor_(awaitor), op_(op){}
            constexpr bool await_ready() const noexcept{ return false;}
            void await_suspend(std::coroutine_handle<> handle) noexcept{
                awaitor_.core_handle_ = handle;
                op_(AwaitorHandler{&awaitor_});
            }
            auto coAwait(IExecutor* executor) const noexcept{
                return *this;
            }
            decltype(auto) await_resume() const noexcept{
                if constexpr(std::is_void_v<Arg>){
                    return;
                } else {
                    return std::move(awaitor_.arg_);
                }
            }

        private:
            Derived &awaitor_;
            Op &op_;
    };
public:
    class AwaitorHandler{
    public:
        AwaitorHandler(Derived *obj):obj_(obj){}
        AwaitorHandler(AwaitorHandler &&) = default;
        AwaitorHandler(const AwaitorHandler &) = default;
        AwaitorHandler &operator=(AwaitorHandler &&) = default;
        AwaitorHandler &operator=(const AwaitorHandler &) = default;
        template<typename... Args>
        void set_value_then_resume(Args &&...args) const{
            set_value(std::forward<Args>(args)...);
            resume();
        }
        template<typename... Args>
        void set_value(Args &&...args) const{
            if constexpr(!std::is_void_v<Arg>){
                obj_->arg_ = {std::forward<Args>(args)...};
            }
        }
        void resume() const{ obj_->core_handle_.resume();}
    private:
        Derived *obj_;
    };
    template<typename Op>
    CallbackAwaitorImpl<Op> coAwait(Op &&op) noexcept{
        return CallbackAwaitorImpl<Op>{static_cast<Derived&>(*this), op};
    }

private:
    std::coroutine_handle<> core_handle_;
  
};

template <typename Arg>
class CallBackAwaitor: public CallbackAwaitroBase<Arg, CallBackAwaitor<Arg>>{
    friend class CallbackAwaitroBase<Arg, CallBackAwaitor<Arg>>;
private:
    Arg arg_;
};
template <>
class CallbackAwaitroBase<void, CallBackAwaitor<void>>{};

// ----------------------------------------------------------------------------
// 对 std::future 适配 await

template <typename T>
class FutureAwaiter{
public:
    using FutureCallBack = std::function<void(std::function<void()>)>;
    FutureAwaiter(std::future<T> &&future):future_(std::move(future)){}
    constexpr bool await_ready() const noexcept{ 
        return future_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }
    void await_suspend(std::coroutine_handle<> handle) noexcept{
        future_.wait_for(std::chrono::milliseconds(0));
        auto resume_func = [handle]() { 
            handle.resume(); 
        };

        if(resume_callback_){
            resume_callback_(resume_func);
        } else if(executor_){  
            executor_->execute(resume_func);
        } else {
            handle.resume();
        }
    }
    auto coAwait(IExecutor* executor) const noexcept{
        return *this;
    }
    decltype(auto) await_resume() noexcept{
        if(future_.valid()){
            return future_.get();
        } else {
            throw std::future_error(std::future_errc::no_state);
        }
    }

    FutureAwaiter& with_future_callback(FutureCallBack &&callback) noexcept{
        resume_callback_ = std::move(callback);
        return *this;
    }

    // 实现 with_executor 函数
    FutureAwaiter with_executor(IExecutor &executor) noexcept{
        executor_ = &executor;
        return *this;
    }
private:
    std::future<T> future_;
    IExecutor *executor_ = nullptr;
    FutureCallBack resume_callback_;
};

template <typename T>
auto operator co_await(std::future<T> &&future){
    return FutureAwaiter<T>(std::move(future));
}

} // namespace coro
} // namespace ToolBox