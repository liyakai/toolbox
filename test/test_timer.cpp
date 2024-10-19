#include "tools/timer.h"
#include "unit_test_frame/unittest.h"
#include <thread>

FIXTURE_BEGIN(Timer)

// class TestTimer : public ToolBox::ITimer
// {
// public:
//     void OnTimer(uint32_t id, uint32_t count) override
//     {
//         fprintf(stderr, "触发ITimer 类型定时器,当前定时器id:%u,count:%u\n", id, count);
//     }
// };

// 测试ITimer触发10次
CASE(TimerCase1)
{
    return;
    ToolBox::TimerMgr->AddTimer([](int times) {
      fprintf(stderr, "触发ITimer 类型定时器,当前定时器触发第 %d 次\n", times);
    }, 1000, 10, __FILE__, __LINE__);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

// 测试ITimer触发无限次
CASE(TimerCase2)
{
    return;
    ToolBox::TimerMgr->AddTimer([](int times) {
      fprintf(stderr, "触发ITimer 类型定时器,当前定时器触发第 %d 次\n", times);
    }, 1000, -1, __FILE__, __LINE__);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

// 测试将 ITimer 对象添加到定时器后立马删除参数的情况.
CASE(TimerCase3)
{
    return;
    struct TimerArgs
    {
        int param1;
        std::string param2;
    };
    auto timer = std::make_shared<TimerArgs>();
    timer->param1 = 100;
    timer->param2 = "参数2";
    // 使用 weak_ptr 来接收参数
    std::weak_ptr<TimerArgs> weak_timer = timer;
    ToolBox::TimerMgr->AddTimer([&weak_timer](int times) {
      auto arg = weak_timer.lock();
      if (arg)
      {
        fprintf(stderr, "触发ITimer 类型定时器,当前定时器触发第 %d 次, param1:%d, param2:%s\n", times, arg->param1, arg->param2.c_str());
      }else
      {
        fprintf(stderr, "触发ITimer 类型定时器,当前定时器触发第 %d 次, 参数已销毁.\n", times);
      }
    }, 1000, 10, __FILE__, __LINE__);
    timer.reset();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

FIXTURE_END(Timer)