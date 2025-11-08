#include "unit_test_frame/unittest.h"
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <functional>
#include <future>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>
#include <string>

#include "tools/cpp20_coroutine.h"

FIXTURE_BEGIN(TestCpp20Coroutine)

template <typename T> struct Generator {
  class ExhaustedException : std::exception {};
  struct promise_type {
    T value;
    bool is_ready = false;
    // 开始执行时直接挂起等待外部调用 resume 获取下一个值
    std::suspend_always initial_suspend() { return {}; }
    // 执行结束后总是挂起, 让 Generator 来销毁
    std::suspend_always final_suspend() noexcept { return {}; }
    // 为了简单,我们认为序列生成器当中不会抛出异常,这里不做任何处理.
    void unhandled_exception() {}

    // 构造协程的返回值类型
    Generator get_return_object() {
      return Generator{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    // 传值的同时要挂起, 值存入value当中.
    std::suspend_always await_transform(T value) {
      this->value = value;
      is_ready = true;
      return {};
    }
    std::suspend_always yield_value(T value) {
      this->value = value;
      is_ready = true;
      return {};
    }
    // 没有返回值
    void return_void() {}
  };
  std::coroutine_handle<promise_type> handle;

  explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
      : handle(handle) {}

  Generator(Generator &&generator) noexcept
      : handle(std::exchange(generator.handle, {})) {}

  Generator(Generator &) = delete;
  Generator &operator=(Generator &) = delete;

  ~Generator() { handle.destroy(); }

  bool has_next() {
    // 协程已经执行完成
    if (handle.done()) {
      return false;
    }
    // 协程还没有执行完成,并且下一个值还没有准备好.
    if (!handle.promise().is_ready) {
      handle.resume();
    }
    if (handle.done()) {
      // 恢复执行之后协程执行完,这时候必然没有通过 co_await 传出值来
      return false;
    } else {
      return true;
    }
  }
  int next() {
    if (has_next()) {
      // 此时一定有值, is_ready 为 true
      // 消费当前的值, 重置 is_ready 为 false
      handle.promise().is_ready = false;
      // 通过 handle 获取 promise,然后再取到 value.
      return handle.promise().value;
    }
    throw ExhaustedException();
  }
  template <typename... TArgs> Generator static from(TArgs... args) {
    (co_yield args, ...);
  }
};

Generator<int32_t> sequence() {
  int i = 0;
  while (i < 5) {
    co_await i++;
  }
}

CASE(TestGenerator) {
  auto generator = sequence();
  for (int i = 0; i < 10; ++i) {
    if (generator.has_next()) {
      fprintf(stderr, "cpp20 coroutine: %d \n", generator.next());
    } else {
      // 协程已经执行完,协程的状态已经销毁.
      break;
    }
  }
}

Generator<int32_t> fibonacci() {
  co_yield 0;
  co_yield 1;
  int32_t a = 0;
  int32_t b = 1;
  while (true) {
    co_yield a + b;
    b = a + b;
    a = b - a;
  }
}

CASE(TestCoroutineFibonacci) {
  auto fibo = fibonacci();
  for (int i = 0; i < 10; ++i) {
    if (fibo.has_next()) {
      fprintf(stderr, "cpp20 coroutine fibonacci: %d \n", fibo.next());
    } else {
      // 协程已经执行完,协程的状态已经销毁.
      break;
    }
  }
}

ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> simple_task2() {
  fprintf(stderr, "cpp20 coroutine. simple_task2 start...\n");

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  fprintf(stderr, "cpp20 coroutine. simple_task2 return after 1s.\n");
  co_return 2;
}
ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> simple_task3() {
  fprintf(stderr, "cpp20 coroutine. simple_task3 start...\n");

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  fprintf(stderr, "cpp20 coroutine. simple_task3 return after 2s.\n");
  co_return 3;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::LooperExecutor> simple_task() {
  fprintf(stderr, "cpp20 coroutine. simple_task start...\n");
  auto result2 = co_await simple_task2();
  fprintf(stderr, "cpp20 coroutine. returns from task2:%d\n", result2);
  auto result3 = co_await simple_task3();
  fprintf(stderr, "cpp20 coroutine. returns from task3:%d\n", result3);
  co_return 1 + result2 + result3;
}

CASE(TestCoroutineTask) {
  auto simpleTask = simple_task();
  fprintf(stderr, "cpp20 coroutine. created simple_task.\n");
  simpleTask
      .then([](int i) -> void {
        fprintf(stderr, "cpp20 coroutine. simple task end:%d\n", i);
      })
      .catching([](std::exception &e) -> void {
        fprintf(stderr, "cpp20 coroutine. error occurred.%s\n", e.what());
      });
  try {
    auto i = simpleTask.get_result();
    fprintf(stderr, "cpp20 coroutine. simple task end from get:%d\n", i);
  } catch (std::exception &e) {
    fprintf(stderr, "cpp20 coroutine. error occurred.%s\n", e.what());
  }
}

void simpleCallback(std::function<void(int32_t x, int32_t y)> callback) {
    fprintf(stderr, "cpp20 coroutine. callback awaitor start...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    fprintf(stderr, "cpp20 coroutine. callback awaitor end...\n");
    callback(1,2);
}

ToolBox::coro::Task<std::pair<int32_t, int32_t>, ToolBox::coro::NewThreadExecutor> test_callback_awaitor() noexcept {
    ToolBox::coro::CallBackAwaitor<std::pair<int32_t,int32_t>> awaitor;
    co_return co_await awaitor.coAwait([](auto handler){
        simpleCallback([handler](int32_t x, int32_t y){
            handler.set_value_then_resume(std::make_pair(x,y));
        });
    }); 
}

CASE(TestCallbackAwaitor){
    fprintf(stderr, "cpp20 coroutine. test callback awaitor start...\n");
    auto task = test_callback_awaitor();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. callback awaitor end:(%d,%d)\n", result.first, result.second);
}

// ============================================================================
// 异常处理测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> task_with_exception() {
    fprintf(stderr, "cpp20 coroutine. task_with_exception start...\n");
    throw std::runtime_error("Test exception in coroutine");
    co_return 42;
}

CASE(TestTaskException) {
    fprintf(stderr, "cpp20 coroutine. test task exception start...\n");
    auto task = task_with_exception();
    
    bool exception_caught = false;
    task.catching([&exception_caught](std::exception &e) {
        fprintf(stderr, "cpp20 coroutine. caught exception in callback: %s\n", e.what());
        exception_caught = true;
    });
    
    try {
        auto result = task.get_result();
        fprintf(stderr, "cpp20 coroutine. unexpected success: %d\n", result);
    } catch (std::exception &e) {
        fprintf(stderr, "cpp20 coroutine. caught exception in get_result: %s\n", e.what());
        exception_caught = true;
    }
    
    // 等待回调执行
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fprintf(stderr, "cpp20 coroutine. exception test completed, caught: %s\n", 
            exception_caught ? "true" : "false");
}

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> nested_task_with_exception() {
    fprintf(stderr, "cpp20 coroutine. nested_task_with_exception start...\n");
    try {
        // 直接 co_await 函数返回值（右值），或者使用 std::move
        auto result = co_await task_with_exception();
        co_return result + 1;
    } catch (...) {
        // 重新抛出异常
        throw;
    }
}

CASE(TestNestedTaskException) {
    fprintf(stderr, "cpp20 coroutine. test nested task exception start...\n");
    auto task = nested_task_with_exception();
    
    try {
        auto result = task.get_result();
        fprintf(stderr, "cpp20 coroutine. unexpected success: %d\n", result);
    } catch (std::exception &e) {
        fprintf(stderr, "cpp20 coroutine. caught nested exception: %s\n", e.what());
    }
}

// ============================================================================
// void 类型 Task 测试用例（当前实现不支持 void，使用 int 代替）
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> void_like_task() {
    fprintf(stderr, "cpp20 coroutine. void_like_task start...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fprintf(stderr, "cpp20 coroutine. void_like_task completed\n");
    co_return 0;  // 返回 0 表示成功
}

CASE(TestVoidTask) {
    fprintf(stderr, "cpp20 coroutine. test void-like task start...\n");
    auto task = void_like_task();
    
    bool finally_called = false;
    task.finally([&finally_called]() {
        fprintf(stderr, "cpp20 coroutine. void-like task finally called\n");
        finally_called = true;
    });
    
    try {
        auto result = task.get_result();
        fprintf(stderr, "cpp20 coroutine. void-like task completed successfully, result: %d\n", result);
    } catch (std::exception &e) {
        fprintf(stderr, "cpp20 coroutine. void-like task error: %s\n", e.what());
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fprintf(stderr, "cpp20 coroutine. void-like task test completed, finally called: %s\n",
            finally_called ? "true" : "false");
}

// ============================================================================
// 不同执行器测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> noop_executor_task() {
    fprintf(stderr, "cpp20 coroutine. noop_executor_task start...\n");
    co_return 100;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::AsyncExecutor> async_executor_task() {
    fprintf(stderr, "cpp20 coroutine. async_executor_task start...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    co_return 200;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::LooperExecutor> looper_executor_task() {
    fprintf(stderr, "cpp20 coroutine. looper_executor_task start...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    co_return 300;
}

CASE(TestDifferentExecutors) {
    fprintf(stderr, "cpp20 coroutine. test different executors start...\n");
    
    // 测试 NoopExecutor
    auto noop_task = noop_executor_task();
    auto noop_result = noop_task.get_result();
    fprintf(stderr, "cpp20 coroutine. noop_executor result: %d\n", noop_result);
    
    // 测试 AsyncExecutor
    auto async_task = async_executor_task();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto async_result = async_task.get_result();
    fprintf(stderr, "cpp20 coroutine. async_executor result: %d\n", async_result);
    
    // 测试 LooperExecutor
    auto looper_task = looper_executor_task();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto looper_result = looper_task.get_result();
    fprintf(stderr, "cpp20 coroutine. looper_executor result: %d\n", looper_result);
    
    fprintf(stderr, "cpp20 coroutine. different executors test completed\n");
}

// ============================================================================
// FutureAwaiter 测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> future_awaiter_task() {
    fprintf(stderr, "cpp20 coroutine. future_awaiter_task start...\n");
    
    // 创建一个异步任务
    auto future = std::async(std::launch::async, []() -> int32_t {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });
    
    // 使用 co_await 等待 future（通过 await_transform 自动转换）
    auto result = co_await ToolBox::coro::FutureAwaiter<int32_t>(std::move(future));
    fprintf(stderr, "cpp20 coroutine. future_awaiter_task got result: %d\n", result);
    co_return result;
}

CASE(TestFutureAwaiter) {
    fprintf(stderr, "cpp20 coroutine. test future awaiter start...\n");
    auto task = future_awaiter_task();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. future awaiter result: %d\n", result);
}

ToolBox::coro::Task<std::string, ToolBox::coro::NoopExecutor> future_awaiter_string_task() {
    fprintf(stderr, "cpp20 coroutine. future_awaiter_string_task start...\n");
    
    auto future = std::async(std::launch::async, []() -> std::string {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return std::string("Hello from future");
    });
    
    auto result = co_await ToolBox::coro::FutureAwaiter<std::string>(std::move(future));
    fprintf(stderr, "cpp20 coroutine. future_awaiter_string_task got result: %s\n", result.c_str());
    co_return result;
}

CASE(TestFutureAwaiterString) {
    fprintf(stderr, "cpp20 coroutine. test future awaiter string start...\n");
    auto task = future_awaiter_string_task();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. future awaiter string result: %s\n", result.c_str());
}

// ============================================================================
// 并发 Task 测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> concurrent_task(int32_t id, int32_t delay_ms) {
    fprintf(stderr, "cpp20 coroutine. concurrent_task[%d] start...\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    fprintf(stderr, "cpp20 coroutine. concurrent_task[%d] completed\n", id);
    co_return id * 10;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::LooperExecutor> concurrent_tasks_test() {
    fprintf(stderr, "cpp20 coroutine. concurrent_tasks_test start...\n");
    
    // 创建多个并发任务
    auto task1 = concurrent_task(1, 100);
    auto task2 = concurrent_task(2, 150);
    auto task3 = concurrent_task(3, 200);
    
    // 顺序等待它们完成（使用 std::move 将左值转换为右值）
    auto result1 = co_await std::move(task1);
    auto result2 = co_await std::move(task2);
    auto result3 = co_await std::move(task3);
    
    fprintf(stderr, "cpp20 coroutine. concurrent_tasks_test results: %d, %d, %d\n", 
            result1, result2, result3);
    co_return result1 + result2 + result3;
}

CASE(TestConcurrentTasks) {
    fprintf(stderr, "cpp20 coroutine. test concurrent tasks start...\n");
    auto task = concurrent_tasks_test();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. concurrent tasks result: %d\n", result);
}

// ============================================================================
// 多个回调组合测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> multi_callback_task() {
    fprintf(stderr, "cpp20 coroutine. multi_callback_task start...\n");
    co_return 999;
}

CASE(TestMultipleCallbacks) {
    fprintf(stderr, "cpp20 coroutine. test multiple callbacks start...\n");
    auto task = multi_callback_task();
    
    int32_t then_count = 0;
    int32_t finally_count = 0;
    
    task.then([&then_count](int32_t value) {
        fprintf(stderr, "cpp20 coroutine. then callback 1: %d\n", value);
        then_count++;
    });
    
    task.then([&then_count](int32_t value) {
        fprintf(stderr, "cpp20 coroutine. then callback 2: %d\n", value);
        then_count++;
    });
    
    task.finally([&finally_count]() {
        fprintf(stderr, "cpp20 coroutine. finally callback called\n");
        finally_count++;
    });
    
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. multiple callbacks result: %d\n", result);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fprintf(stderr, "cpp20 coroutine. then_count: %d, finally_count: %d\n", 
            then_count, finally_count);
}

// ============================================================================
// CallbackAwaitor void 版本测试用例（使用 int 代替 void）
// ============================================================================

void voidCallback(std::function<void()> callback) {
    fprintf(stderr, "cpp20 coroutine. void callback awaitor start...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fprintf(stderr, "cpp20 coroutine. void callback awaitor end...\n");
    callback();
}

ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> test_void_callback_awaitor() noexcept {
    ToolBox::coro::CallBackAwaitor<int32_t> awaitor;
    auto result = co_await awaitor.coAwait([](auto handler){
        voidCallback([handler](){
            handler.set_value_then_resume(0);  // 返回 0 表示成功
        });
    });
    co_return result;
}

CASE(TestVoidCallbackAwaitor){
    fprintf(stderr, "cpp20 coroutine. test void callback awaitor start...\n");
    auto task = test_void_callback_awaitor();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. void callback awaitor completed, result: %d\n", result);
}

// ============================================================================
// 复杂嵌套场景测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> inner_task(int32_t value) {
    fprintf(stderr, "cpp20 coroutine. inner_task[%d] start...\n", value);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    co_return value * 2;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::LooperExecutor> middle_task(int32_t value) {
    fprintf(stderr, "cpp20 coroutine. middle_task[%d] start...\n", value);
    auto inner1 = inner_task(value);
    auto inner2 = inner_task(value + 1);
    // 使用 std::move 将左值转换为右值
    auto result1 = co_await std::move(inner1);
    auto result2 = co_await std::move(inner2);
    co_return result1 + result2;
}

ToolBox::coro::Task<int32_t, ToolBox::coro::LooperExecutor> outer_task() {
    fprintf(stderr, "cpp20 coroutine. outer_task start...\n");
    auto middle1 = middle_task(10);
    auto middle2 = middle_task(20);
    // 使用 std::move 将左值转换为右值
    auto result1 = co_await std::move(middle1);
    auto result2 = co_await std::move(middle2);
    fprintf(stderr, "cpp20 coroutine. outer_task results: %d, %d\n", result1, result2);
    co_return result1 + result2;
}

CASE(TestNestedTasks) {
    fprintf(stderr, "cpp20 coroutine. test nested tasks start...\n");
    auto task = outer_task();
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. nested tasks result: %d\n", result);
}

// ============================================================================
// SharedLooperExecutor 测试用例
// ============================================================================

ToolBox::coro::Task<int32_t> shared_looper_task(int32_t id) {
    fprintf(stderr, "cpp20 coroutine. shared_looper_task[%d] start...\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    co_return id * 100;
}

CASE(TestSharedLooperExecutor) {
    fprintf(stderr, "cpp20 coroutine. test shared looper executor start...\n");
    auto task1 = shared_looper_task(1);
    auto task2 = shared_looper_task(2);
    auto task3 = shared_looper_task(3);
    
    auto result1 = task1.get_result();
    auto result2 = task2.get_result();
    auto result3 = task3.get_result();
    
    fprintf(stderr, "cpp20 coroutine. shared looper results: %d, %d, %d\n", 
            result1, result2, result3);
}

// ============================================================================
// 链式调用测试用例
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NoopExecutor> chain_task() {
    fprintf(stderr, "cpp20 coroutine. chain_task start...\n");
    co_return 123;
}

CASE(TestChainCallbacks) {
    fprintf(stderr, "cpp20 coroutine. test chain callbacks start...\n");
    auto task = chain_task();
    
    task.then([](int32_t value) {
        fprintf(stderr, "cpp20 coroutine. chain then 1: %d\n", value);
    }).then([](int32_t value) {
        fprintf(stderr, "cpp20 coroutine. chain then 2: %d\n", value);
    }).catching([](std::exception &e) {
        fprintf(stderr, "cpp20 coroutine. chain catch: %s\n", e.what());
    }).finally([]() {
        fprintf(stderr, "cpp20 coroutine. chain finally\n");
    });
    
    auto result = task.get_result();
    fprintf(stderr, "cpp20 coroutine. chain callbacks result: %d\n", result);
}

// ============================================================================
// 性能测试用例 - 大量并发任务
// ============================================================================

ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor> perf_task(int32_t id) {
    co_return id;
}

CASE(TestPerformanceManyTasks) {
    fprintf(stderr, "cpp20 coroutine. test performance many tasks start...\n");
    const int32_t task_count = 100;
    std::vector<ToolBox::coro::Task<int32_t, ToolBox::coro::NewThreadExecutor>> tasks;
    
    for (int32_t i = 0; i < task_count; ++i) {
        tasks.push_back(perf_task(i));
    }
    
    int32_t sum = 0;
    for (auto &task : tasks) {
        sum += task.get_result();
    }
    
    fprintf(stderr, "cpp20 coroutine. performance test completed, sum: %d\n", sum);
    fprintf(stderr, "cpp20 coroutine. expected sum: %d\n", task_count * (task_count - 1) / 2);
}

FIXTURE_END(TestCpp20Coroutine)