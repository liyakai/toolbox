#include "src/tools/timer.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(Timer)

class TestTimer : public ITimer
{
public:
    void OnTimer(uint32_t id, uint32_t count) override
    {
        fprintf(stderr,"触发ITimer 类型定时器,当前定时器id:%u,count:%u\n",id, count);
    }
};

// 测试ITimer触发10次
CASE(TimerCase1){
    static TestTimer timer;
    TimerMgr->AddTimer(&timer, 10086, 1000, 10, __FILE__, __LINE__);
    while(false) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        TimerMgr->Update();
    }
}

// 测试ITimer触发无限次
CASE(TimerCase2){
    static TestTimer timer;
    TimerMgr->AddTimer(&timer, 10087, 1000, -1, __FILE__, __LINE__);
    while(false) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        TimerMgr->Update();
    }
}

class LambdaArgs : public IArgs
{
public:
    int32_t arg1 = 0;
    std::string arg2;
};

// 测试回调
CASE(TimerCase3){
    static auto lambda = [](IArgs* iargs, void* arg)->bool{
        if(nullptr == iargs)
        {
            fprintf(stderr,"参数 iargs 为空指针.\n");
            return false;
        }
        auto* largs = dynamic_cast<LambdaArgs*>(iargs);
        if(nullptr == largs)
        {
            fprintf(stderr,"参数 iargs 向子类转换失败.\n");
            return false;
        }
        fprintf(stderr,"触发 回调 类型定时器 lambda.参数为 arg1:%d,arg2:%s.\n", largs->arg1, largs->arg2.c_str());
        return true;
    };
    static LambdaArgs largs;
    largs.arg1 = 100;
    largs.arg2 = "参数2";
    TimerMgr->AddTimer(lambda, &largs, nullptr, 1000, -1, __FILE__, __LINE__);
    while(false) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        TimerMgr->Update();
    }
}

class TestTimerDelegate
{
public:
    bool OnTimer(IArgs* iargs, void* args)
    {
                if(nullptr == iargs)
        {
            fprintf(stderr,"参数 iargs 为空指针.\n");
            return false;
        }
        auto* largs = dynamic_cast<LambdaArgs*>(iargs);
        if(nullptr == largs)
        {
            fprintf(stderr,"参数 iargs 向子类转换失败.\n");
            return false;
        }
        fprintf(stderr,"触发 Delegate 类型定时器 lambda.参数为 arg1:%d,arg2:%s.\n", largs->arg1, largs->arg2.c_str());
        return true;
    }
};

// 测试委托
CASE(TimerCase4){
    static TestTimerDelegate timer;
    static LambdaArgs largs;
    largs.arg1 = 100;
    largs.arg2 = "参数2";
    TimerMgr->AddTimer(DelegateCombination(TestTimerDelegate, OnTimer, &timer), &largs, nullptr, 1000, -1, __FILE__, __LINE__);
    while(false) // 打开测试将这里改为 true
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        TimerMgr->Update();
    }
}

FIXTURE_END(Timer)