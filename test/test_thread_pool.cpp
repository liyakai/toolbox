#include "tools/thread_pool.h"
#include "unit_test_frame/unittest.h"
#include <map>

FIXTURE_BEGIN(TestThreadPool)

// 测试数字有返回
CASE(TestThreadPoolCase1_num)
{
    std::mutex mtx;
    try
    {
        ToolBox::ThreadPool thread_pool;
        std::vector<std::future<int32_t>> v_future;
        for (int32_t i = 0; i < 10; i++)
        {
            auto res = thread_pool.Add([](int32_t answer)->int32_t{ return answer;}, i);
            v_future.emplace_back(std::move(res));
        }
        for (size_t i = 0; i < v_future.size(); i++)
        {
            std::lock_guard<std::mutex> lg(mtx);
            fprintf(stderr, "threadpool result: %d\n", v_future[i].get());
        }
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[TestThreadPoolCase] %s \n", e.what());
    }
}

// 测试字符串无返回
CASE(TestThreadPoolCase1_num_str)
{
    std::mutex mtx;
    try
    {
        ToolBox::ThreadPool thread_pool;
        std::vector<std::future<void>> v_future;
        for (int32_t i = 0; i <= 5; i++)
        {
            auto res = thread_pool.Add([&mtx](const std::string & str1, const std::string & str2)
            {
                std::lock_guard<std::mutex> lg(mtx);
                fprintf(stderr, "%s%s\n", str1.c_str(), str2.c_str());
            }, "hello ", "world");
            v_future.emplace_back(std::move(res));
        }
        for (size_t i = 0; i < v_future.size(); i++)
        {
            v_future[i].get();
        }


    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[TestThreadPoolCase] %s \n", e.what());
    }
}

FIXTURE_END(TestThreadPool)