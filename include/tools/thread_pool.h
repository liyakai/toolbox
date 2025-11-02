#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <stdint.h>

namespace ToolBox{

/*
*  定义线程池
*/
class ThreadPool
{
public: 
    typedef std::function<void()> task_type;
    /*
    * 构造
    */
    explicit ThreadPool(int32_t n = 0);
    /*
    * 析构
    */
    ~ThreadPool()
    {
        Stop();
        cond_.notify_all();
        for (auto& thread : threads_)
        {
            if (thread.joinable())
            {
                thread.join();
            } 
        }
    }
    /*
    * 停止线程池
    */
    void Stop()
    {
        stop_.store(true, std::memory_order_release);
    }
    /*
    * 增加任务
    */
    template<typename Function, typename... Args>
    std::future<typename std::invoke_result_t<Function, Args...>> Add(Function&&, Args&&...);
private:
    ThreadPool(ThreadPool&& tp) = delete;
    ThreadPool& operator = (ThreadPool&& tp) = delete;

    ThreadPool(const ThreadPool& tp) = delete;
    ThreadPool& operator = (const ThreadPool&& tp) = delete;

private:
    /// 原子量: 是否停止线程池
    std::atomic<bool> stop_;
    /// 互斥锁
    std::mutex mtx_;
    /// 条件变量,阻塞工作线程,以及通知有任务产生
    std::condition_variable cond_;
    /// 任务队列
    std::queue<task_type> tasks_;
    /// 线程池
    std::vector<std::thread> threads_;
};

inline ThreadPool::ThreadPool(int32_t n)
    : stop_(false)
    {
        int32_t nthreads = n;
        if(nthreads <= 0)
        {
            nthreads = std::thread::hardware_concurrency();
            nthreads = (nthreads == 0 ? 2 : nthreads);
        }
        for (int32_t i = 0; i < nthreads; i++)
        {
            threads_.emplace_back(std::thread([this]{
                while (!stop_.load(std::memory_order_acquire))
                {
                    task_type task;
                    {
                        std::unique_lock<std::mutex> ulk(this->mtx_);
                        this->cond_.wait(ulk, [this]{ return stop_.load(std::memory_order_acquire) || (!this->tasks_.empty()); });
                        if(stop_.load(std::memory_order_acquire))
                        {
                            return;
                        }
                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    task();
                }
                
            }));
        }
    }

template<typename Function, typename... Args>
std::future<typename std::invoke_result_t<Function, Args...>>
ThreadPool::Add(Function&& func, Args&&... args)
{
    using return_type = typename std::invoke_result_t<Function, Args...>;
    using task = std::packaged_task<return_type()>;

    auto tk = std::make_shared<task>(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    auto ret = tk->get_future();
    {
        std::lock_guard<std::mutex> lg(mtx_);
        if(stop_.load(std::memory_order_acquire))
        {
            throw std::runtime_error("thread pool has stopped.");
        }
        tasks_.emplace([tk]{(*tk)();});
    }
    cond_.notify_one();
    return ret;
}


};  // ToolBox