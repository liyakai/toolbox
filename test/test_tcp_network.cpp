#include "src/network/network_mgr.h"
#include "unit_test_frame/unittest.h"

class TestNetworkMaster : public NetworkMaster, public DebugPrint
{
    void OnAccepted(uint64_t conn_id) override 
    {
        if(GetDebugStatus())
        {
            Print("接到客户端连接,连接ID:%llu\n", conn_id);
        }
    };
    void OnReceived(uint64_t conn_id, const char* data, size_t size) override
    {
        if(GetDebugStatus())
        {
            Print("收到客户端数据长度为%d\n", size);
            PrintData(data, 32);
        }
        Send(NT_TCP, conn_id, data, size);
    };
    void OnClose(uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) override
    {
        if(GetDebugStatus())
        {
            Print("断开与客户端之间的连接,连接ID:%ul\n", conn_id);
        }
    }
};

FIXTURE_BEGIN(TCPNetWork)

CASE(test_tcp)
{
    Singleton<TestNetworkMaster>::Instance()->SetDebugPrint(true);
    Singleton<TestNetworkMaster>::Instance()->Accept("127.0.0.1", 9500, NT_TCP);
    Singleton<TestNetworkMaster>::Instance()->Start();
    bool run = true;
    std::thread t([&](){
        while(run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Singleton<TestNetworkMaster>::Instance()->Update();
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