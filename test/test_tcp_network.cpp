#include "src/network/network_mgr.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TCPNetWork)

CASE(test_tcp)
{
    Singleton<NetworkMaster>::Instance()->Accept("127.0.0.1", 9500, NT_TCP);
    Singleton<NetworkMaster>::Instance()->Start();
    bool run = true;
    std::thread t([&](){
        while(run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Singleton<NetworkMaster>::Instance()->Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    while(true)
    {
        if(used_time > 1000*1000) break;
        uint32_t time_left = (1000*1000 - used_time)/1000;
        if(time_left != old_time)
        {
            old_time = time_left;
            fprintf(stderr,"距离网络库停止还有%d秒\n",time_left);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    run = false;
    t.join();
    Singleton<NetworkMaster>::Instance()->StopWait();
    return ;
}

FIXTURE_END(TCPNetWork)