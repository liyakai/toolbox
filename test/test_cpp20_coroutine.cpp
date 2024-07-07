#include "unit_test_frame/unittest.h"
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <future>
#include <coroutine>
#include <exception>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "tools/cpp20_coroutine.h"


FIXTURE_BEGIN(TestCpp20Coroutine)

template<typename T>
struct Generator
{
    class ExhaustedException : std::exception {};
    struct promise_type
    {
        T value;
        bool is_ready = false;
        // 开始执行时直接挂起等待外部调用 resume 获取下一个值
        std::suspend_always initial_suspend()
        {
            return {};
        }
        // 执行结束后总是挂起, 让 Generator 来销毁
        std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        // 为了简单,我们认为序列生成器当中不会抛出异常,这里不做任何处理.
        void unhandled_exception() {}

        // 构造协程的返回值类型
        Generator get_return_object()
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        // 传值的同时要挂起, 值存入value当中.
        std::suspend_always await_transform(T value)
        {
            this->value = value;
            is_ready = true;
            return {};
        }
        std::suspend_always yield_value(T value)
        {
            this->value = value;
            is_ready = true;
            return {};
        }
        // 没有返回值
        void return_void() {}

    };
    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> handle) noexcept : handle(handle) {}

    Generator(Generator&& generator) noexcept: handle(std::exchange(generator.handle, {})) {}

    Generator(Generator&) = delete;
    Generator& operator=(Generator&) = delete;

    ~Generator()
    {
        handle.destroy();
    }

    bool has_next()
    {
        // 协程已经执行完成
        if (handle.done())
        {
            return false;
        }
        // 协程还没有执行完成,并且下一个值还没有准备好.
        if (!handle.promise().is_ready)
        {
            handle.resume();
        }
        if (handle.done())
        {
            // 恢复执行之后协程执行完,这时候必然没有通过 co_await 传出值来
            return false;
        }
        else
        {
            return true;
        }
    }
    int next()
    {
        if (has_next())
        {
            // 此时一定有值, is_ready 为 true
            // 消费当前的值, 重置 is_ready 为 false
            handle.promise().is_ready = false;
            // 通过 handle 获取 promise,然后再取到 value.
            return handle.promise().value;
        }
        throw  ExhaustedException();
    }
    template<typename ...TArgs>
    Generator static from(TArgs ...args)
    {
        (co_yield args, ...);
    }

};

Generator<int32_t> sequence()
{
    int i = 0;
    while (i < 5)
    {
        co_await i++;
    }
}


CASE(TestGenerator)
{
    auto generator = sequence();
    for (int i = 0; i < 10; ++i)
    {
        if (generator.has_next())
        {
            fprintf(stderr, "cpp20 coroutine: %d \n", generator.next());
        }
        else
        {
            // 协程已经执行完,协程的状态已经销毁.
            break;
        }
    }
}

Generator<int32_t> fibonacci()
{
    co_yield 0;
    co_yield 1;
    int32_t a = 0;
    int32_t b = 1;
    while (true)
    {
        co_yield a + b;
        b = a + b;
        a = b - a;
    }
}

CASE(TestCoroutineFibonacci)
{
    auto fibo = fibonacci();
    for (int i = 0; i < 10; ++i)
    {
        if (fibo.has_next())
        {
            fprintf(stderr, "cpp20 coroutine fibonacci: %d \n", fibo.next());
        }
        else
        {
            // 协程已经执行完,协程的状态已经销毁.
            break;
        }
    }
}

Task<int32_t, NewThreadExecutor> simple_task2()
{
    fprintf(stderr, "cpp20 coroutine. simple_task2 start...\n");

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    fprintf(stderr, "cpp20 coroutine. simple_task2 return after 1s.\n");
    co_return 2;
}
Task<int32_t, NewThreadExecutor> simple_task3()
{
    fprintf(stderr, "cpp20 coroutine. simple_task3 start...\n");

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    fprintf(stderr, "cpp20 coroutine. simple_task3 return after 2s.\n");
    co_return 3;
}

Task<int32_t, NewThreadExecutor> simple_task()
{
    fprintf(stderr, "cpp20 coroutine. simple_task start...\n");
    auto result2 = co_await simple_task2();
    fprintf(stderr, "cpp20 coroutine. returns from task2:%d\n", result2);
    auto result3 = co_await simple_task3();
    fprintf(stderr, "cpp20 coroutine. returns from task3:%d\n", result3);
    co_return 1 + result2 + result3;
}


CASE(TestCoroutineTask)
{
    auto simpleTask = simple_task();
    simpleTask.then([](int i)->void
    {
        fprintf(stderr, "cpp20 coroutine. simple task end:%d\n", i);
    }).catching([](std::exception & e)->void
    {
        fprintf(stderr, "cpp20 coroutine. error occurred.%s\n", e.what());
    });
    try
    {
        auto i = simpleTask.get_result();
        fprintf(stderr, "cpp20 coroutine. simple task end from get:%d\n", i);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "cpp20 coroutine. error occurred.%s\n", e.what());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100 * 1000));
}



FIXTURE_END(TestCpp20Coroutine)