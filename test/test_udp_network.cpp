#include "src/network/network_mgr.h"
#include "unit_test_frame/unittest.h"
#include <gperftools/profiler.h>


class TestNetworkEcho : public NetworkMaster, public DebugPrint
{
public:
    void OnAccepted(uint64_t conn_id) override 
    {
        Print("接到客户端连接,连接ID:%llu\n", conn_id);
    };
    void OnConnected(uint64_t conn_id) override
    {
        Print("主动连接成功:%llu\n", conn_id);
    };
    void OnConnectedFailed(ENetErrCode err_code, int32_t err_no) override
    {
        Print("连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
    };
    void OnErrored(uint64_t conn_id, ENetErrCode err_code, int32_t err_no) override
    {
        Print("发生错误, connect_id：%lu 错误码:%d, 系统错误码:%d\n", conn_id,  err_code, err_no);
    }
    void OnReceived(uint64_t conn_id, const char* data, size_t size) override
    {
        Print("收到客户端数据长度为%d,conn_id:%lu\n", size, conn_id);
        // PrintData(data, 16);
        Send(NT_UDP, conn_id, data, size);
    };
    void OnClose(uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) override
    {
        Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
    }
};



FIXTURE_BEGIN(UdpEpollNetwork)

CASE(test_udp_echo)
{
    //return;
    // ProfilerStart("test_udp_echo.prof");
    fprintf(stderr,"网络库测试用例: test_udp_echo \n");
    Singleton<TestNetworkEcho>::Instance()->SetDebugPrint(true);
    Singleton<TestNetworkEcho>::Instance()->Accept("127.0.0.1", 9700, NT_UDP, 10*1024*1024, 10*1024*1024);
    Singleton<TestNetworkEcho>::Instance()->Start();
    bool run = true;
    std::thread t([&](){
        while(run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Singleton<TestNetworkEcho>::Instance()->Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 24*3600*1000;
    while(true)
    {
        if(used_time > run_mill_seconds) break;
        uint32_t time_left = (run_mill_seconds - used_time)/1000;
        if(time_left != old_time)
        {
            if(time_left + 10 <= old_time || old_time == 0)
            {
                fprintf(stderr,"距离网络库停止还有%d秒\n",time_left);
                old_time = time_left;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    run = false;
    Singleton<TestNetworkEcho>::Instance()->StopWait();
    t.join();
    ProfilerStop();
    return ;
}

FIXTURE_END(UdpEpollNetwork)

