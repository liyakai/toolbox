#include "tools/timer.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(Timer)

class TestTimer : public ToolBox::ITimer
{
public:
    void OnTimer(uint32_t id, uint32_t count) override
    {
        fprintf(stderr, "触发ITimer 类型定时器,当前定时器id:%u,count:%u\n", id, count);
    }
};

// 测试ITimer触发10次
CASE(TimerCase1)
{
    return;
    auto timer = std::make_shared<TestTimer>();
    ToolBox::TimerMgr->AddTimer(timer, 10086, 1000, 10, __FILE__, __LINE__);
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
    auto timer = std::make_shared<TestTimer>();
    ToolBox::TimerMgr->AddTimer(timer, 10087, 1000, -1, __FILE__, __LINE__);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

// 测试将 ITimer 对象添加到定时器后立马 ITimer 对象的情况.
CASE(TimerCase3)
{
    return;
    auto timer = std::make_shared<TestTimer>();
    ToolBox::TimerMgr->AddTimer(timer, 10087, 1000, 10, __FILE__, __LINE__);
    timer.reset();
    while (false) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        fprintf(stderr, "[定时器测试3] TimerMgr->Update() \n");
        ToolBox::TimerMgr->Update();
    }
}

class LambdaArgs : public ToolBox::IArgs
{
public:
    int32_t arg1 = 0;
    std::string arg2;
};
static auto lambda = [](std::weak_ptr<ToolBox::IArgs> iargs, std::weak_ptr<void> arg)->bool
{
    auto args = iargs.lock();
    if (nullptr == args)
    {
        fprintf(stderr, "[lambda]参数 iargs 为空指针.\n");
        return false;
    }
    auto* largs = dynamic_cast<LambdaArgs*>(args.get());
    if (nullptr == largs)
    {
        fprintf(stderr, "[lambda]参数 args 向子类转换失败.\n");
        return false;
    }
    fprintf(stderr, "触发 回调 类型定时器 lambda.参数为 arg1:%d,arg2:%s.\n", largs->arg1, largs->arg2.c_str());
    return true;
};
// 测试回调lambda
CASE(TimerCase4)
{
    return;
    auto largs = std::make_shared<LambdaArgs>();
    largs->arg1 = 100;
    largs->arg2 = "参数2";
    ToolBox::TimerMgr->AddTimer(lambda, largs, std::weak_ptr<void>(), 1000, -1, __FILE__, __LINE__);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

// 测试回调lambda 参数AddTimer之后立马重置
CASE(TimerCase5)
{
    return;
    auto largs = std::make_shared<LambdaArgs>();
    largs->arg1 = 100;
    largs->arg2 = "参数2";
    ToolBox::TimerMgr->AddTimer(lambda, largs, std::weak_ptr<void>(), 1000, -1, __FILE__, __LINE__);
    largs.reset();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

class TestTimerDelegate
{
public:
    bool OnTimer(std::weak_ptr<ToolBox::IArgs> iargs, std::weak_ptr<void> arg)
    {
        auto args = iargs.lock();
        if (nullptr == args)
        {
            fprintf(stderr, "[TestTimerDelegate]参数 iargs 为空指针.\n");
            return false;
        }
        auto* largs = dynamic_cast<LambdaArgs*>(args.get());
        if (nullptr == largs)
        {
            fprintf(stderr, "[TestTimerDelegate]参数 args 向子类转换失败.\n");
            return false;
        }
        fprintf(stderr, "触发 Delegate 类型定时器 .参数为 arg1:%d,arg2:%s.\n", largs->arg1, largs->arg2.c_str());
        return true;
    }
};

// 测试委托
CASE(TimerCase6)
{
    return;
    auto timer = std::make_shared<TestTimerDelegate>();
    auto largs = std::make_shared<LambdaArgs>();
    largs->arg1 = 100;
    largs->arg2 = "参数2";
    ToolBox::TimerMgr->AddTimer(DelegateCombination(TestTimerDelegate, OnTimer, timer), largs, std::weak_ptr<void>(), 1000, -1, __FILE__, __LINE__);
    while (true) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

// 测试委托 测试对象和参数在AddTimer后立马重置
CASE(TimerCase7)
{
    return;
    auto timer = std::make_shared<TestTimerDelegate>();
    auto largs = std::make_shared<LambdaArgs>();
    largs->arg1 = 100;
    largs->arg2 = "参数2";
    ToolBox::TimerMgr->AddTimer(DelegateCombination(TestTimerDelegate, OnTimer, timer), largs, std::weak_ptr<void>(), 1000, -1, __FILE__, __LINE__);
    timer.reset();
    largs.reset();
    while (true) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ToolBox::TimerMgr->Update();
    }
}

FIXTURE_END(Timer)