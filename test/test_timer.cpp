#include "src/tools/timer.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(Timer)

class FrameTime
{
public:
    uint64_t GetMillSecond()
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch());
        return ms.count();
    }
    int64_t GetDeltaTimeMillSeconds()
    {
        return frame_time - last_frame_time;
    }
    void UpdateFrameTime()
    {
        last_frame_time = frame_time;
        frame_time = GetMillSecond();
    }
    void ProcessUpdate()
    {
        TimerMgr->Update();
    }

private:
    int64_t delta_time = 0;
    int64_t frame = 50;
    uint64_t frame_time = 0;
    uint64_t last_frame_time = 0;
};

class TestTimer : public ITimer
{
public:
    void OnTimer(uint32_t id, uint32_t count) override
    {
        fprintf(stderr,"触发定时器,当前定时器id:%u,count:%u\n",id, count);
    }
};


CASE(TimerCase1){
    FrameTime frame_time;
    TestTimer timer;
    TimerMgr->AddTimer(&timer, 10086, 2000, 10, __FILE__, __LINE__);
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame_time.UpdateFrameTime();
        frame_time.ProcessUpdate();
    }


}

FIXTURE_END(Timer)