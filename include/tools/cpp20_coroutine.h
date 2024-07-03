#pragma once



template<typename T>
struct Result
{
    // 初始化为默认值
    explicit Result() = default;

    // 当 Task 正常返回是用结果初始化 Result
    explicit Result(T&& value) : _value(value) {}

    // 当 Task 抛异常时用异常初始化 Result
    explicit Result(std::exception_ptr&& exception_ptr): _exception_ptr(exception_ptr) {}

    // 读取结果,有异常则抛出异常
    T get_or_throw()
    {
        if (_exception_ptr)
        {
            std::rethrow_exception(_exception_ptr);
        }
        return _value;
    }

private:
    T _value{};
    std::exception_ptr _exception_ptr;
};

template <typename ResultType>
struct TaskPromise;

template<typename ResultType>
struct Task
{
    // 声明 promise_type 为 TaskPromise 类型
    using promise_type = TaskPromise<ResultType>;

    ResultType get_result()
    {
        return handle.promise().get_result();
    }
    Task& then(std::function<void(ResultType)>&& func)
    {
        // 将回调函数 func 注册到 promise 中.
        handle.promise().on_completed([func](auto result)
        {
            try
            {
                func(result.get_or_throw());
            }
            catch (std::exception& e)
            {
                // 忽略异常
            }
        });
        return *this;
    }
    Task& catching(std::function<void(std::exception&)>&& func)
    {
        handle.promise().on_completed([func](auto result)
        {
            try
            {
                // 忽略返回值
                result.get_or_throw();
            }
            catch (std::exception& e)
            {
                func(e);
            }
        });
        return *this;
    }

    Task& finally(std::function<void()>&& func)
    {
        handle.promise().on_completed([func](auto result)
        {
            func();
        });
        return *this;
    }
    explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}
    Task(Task&& task) noexcept : handle(std::exchange(task.handle, {})) {}
    Task(Task&) = delete;
    Task& operator=(Task&) = delete;
    ~Task()
    {
        if (handle)
        {
            handle.destroy();
        }
    }
private:
    std::coroutine_handle<promise_type> handle;
};


template<typename R>
struct TaskAwaiter
{
    explicit TaskAwaiter(Task<R>&& task) noexcept: task(std::move(task)) {}
    TaskAwaiter(TaskAwaiter&& completion) noexcept: task(std::exchange(completion.task, {})) {}
    TaskAwaiter(TaskAwaiter&) = delete;
    TaskAwaiter& operator=(TaskAwaiter&) = delete;
    // 必要的法定函数.
    constexpr bool await_ready() const noexcept
    {
        return false;
    }
    // 必要的法定函数.
    void await_suspend(std::coroutine_handle<> handle) noexcept
    {
        // 当 task 执行完之后调用 resume
        task.finally([handle]()
        {
            handle.resume();
        });
    }
    // 协程恢复执行时,被等待的Task 已经执行完,调用 get_result 来获取结果
    // await_resume 的返回值类型也是不限定的，返回值将作为 co_await 表达式的返回值
    R await_resume() noexcept
    {
        return task.get_result();
    }
private:
    Task<R> task;       // 二阶Task,即协程体内部的 co_await 获得的 task.
};

template <typename ResultType>
struct TaskPromise
{
    // 构造协程的返回值对象 Task [此函数是法定构建 Result 对象的函数]
    // 协程的状态被创建出来之后，会立即构造 promise_type 对象，进而调用 get_return_object 来创建返回值对象
    Task<ResultType> get_return_object()
    {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }


    // 法定函数. 协程立即执行
    // 协程体执行的第一步是调用 co_await promise.initial_suspend()，initial_suspend 的返回值就是一个等待对象（awaiter）
    std::suspend_never initial_suspend()
    {
        return {};
    }

    // 法定函数. 执行结束后挂起,等待外部销毁,该逻辑与前面的 Generator 类似
    std::suspend_always final_suspend() noexcept
    {
        return {};
    }


    // 法定函数.
    void unhandled_exception()
    {
        std::lock_guard lock(completion_lock);
        // 将异常存入 result
        result = Result<ResultType>(std::current_exception());
        // 通知 get_result 当中的 wait
        completion.notify_all();
    }

    // 法定函数. co_return 10000; 1000 会作为参数传入, 即 return_value 函数的参数 value 的值为 1000
    // void 版本为 return_void
    void return_value(ResultType value)
    {
        std::lock_guard lock(completion_lock);
        // 将返回值存入 result, 对应于协程内部的 'co_return value'
        result = Result<ResultType>(std::move(value));
        // 通知 get_result 当中的 wait
        completion.notify_all();
    }
    // 自定义函数,获取结果,如果还没有结果就等待.
    ResultType get_result()
    {
        // 如果 result 没有值, 说明协程还没有运行完,等待值被写入再返回.
        std::unique_lock lock(completion_lock);
        if (!result.has_value())
        {
            // 等待写入值之后调用 notify_all
            completion.wait(lock);
        }
        return result->get_or_throw();
    }

    // 定义了 await_transform 函数之后，co_await expr 就相当于 co_await promise.await_transform(expr) 了
    template<typename _ResultType>
    TaskAwaiter<_ResultType> await_transform(Task<_ResultType>&& task)
    {
        return TaskAwaiter<_ResultType>(std::move(task));
    }

    void on_completed(std::function<void(Result<ResultType>)>&& func)
    {
        std::unique_lock lock(completion_lock);
        // 加锁判断 result
        if (result.has_value())
        {
            // result 已经有值.
            auto value = result.value();
            // 解锁之后再调用 func
            lock.unlock();
            func(value);
        }
        else
        {
            // 否则添加回调函数,等待调用
            completion_callbacks.push_back(func);
        }
    }

private:
    // 使用 std::optional 可以区分协程是否执行完成.
    std::optional<Result<ResultType>> result;

    std::mutex completion_lock;
    std::condition_variable completion;

    // 回调列表,允许对同一个 Task 添加多个回调.
    std::list<std::function<void(Result<ResultType>)>> completion_callbacks;

    void notify_callbacks()
    {
        auto value = result.value();
        for (auto& callback : completion_callbacks)
        {
            callback(value);
        }
        // 调用完成,清空回调
        completion_callbacks.clear();
    }
};