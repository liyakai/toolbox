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
        int64_t delta = GetDeltaTimeMillSeconds();
        delta_time += delta;
        if(delta_time >= frame)
        {
            delta_time = delta < frame ? delta_time - frame : 0;
            TimerMgr->Update(delta);
        }
    }

private:
    int64_t delta_time = 0;
    int64_t frame = 50;
    uint64_t frame_time = 0;
    uint64_t last_frame_time = 0;
};




CASE(TimerCase1){
    FrameTime frame_time;
    TimerMgr->AddTimer();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        frame_time.UpdateFrameTime();
        frame_time.ProcessUpdate();
    }


}

FIXTURE_END(Timer)