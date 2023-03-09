#include "network/network_channel.h"
#include "unit_test_frame/unittest.h"
#ifdef USE_GPERF_TOOLS
#include <gperftools/profiler.h>
#endif // USE_GPERF_TOOLS

class TestUdpNetworkEcho : public ToolBox::NetworkChannel, public ToolBox::DebugPrint
{
public:
    void OnBinded(ToolBox::NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port) override
    {
        Print("[TestNetworkEcho] 服务器已建立监听端口,网络类型:%d, 连接ID:%llu, ip:%s, port:%d\n", type, conn_id, ip.c_str(), port);
    };
    void OnAcceptting(ToolBox::NetworkType type, int32_t fd) override
    {
        Print("[TestNetworkEcho] 正准备将新连接加入io多路复用,网络类型:%d fd:%d\n", type, fd);
    };
    void OnAccepted(ToolBox::NetworkType type, uint64_t conn_id) override
    {
        Print("[TestNetworkEcho] 接到客户端连接,连接ID:%llu\n", conn_id);
    };
    void OnConnected(ToolBox::NetworkType type, uint64_t conn_id) override
    {
        Print("[TestNetworkEcho] 主动连接成功:%llu\n", conn_id);
    };
    void OnConnectedFailed(ToolBox::NetworkType type, ToolBox::ENetErrCode err_code, int32_t err_no) override
    {
        Print("[TestNetworkEcho] 连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
    };
    void OnErrored(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) override
    {
        Print("[TestNetworkEcho] 发生错误, connect_id:%lu 错误码:%d, 系统错误码:%d\n", conn_id,  err_code, err_no);
    }
    void OnReceived(ToolBox::NetworkType type, uint64_t conn_id, const char* data, size_t size) override
    {
        // Print("收到客户端数据长度为%d,conn_id:%lu\n", size, conn_id);
        // PrintData(data, 16);
        Send(ToolBox::NT_UDP, conn_id, data, size);
    };
    void OnClose(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err) override
    {
        Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
    }
};


class TestUdpNetworkForward : public ToolBox::NetworkChannel, public ToolBox::DebugPrint
{
public:
    void OnBinded(ToolBox::NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port) override
    {
        Print("[TestUdpNetworkForward] 服务器已建立监听端口,网络类型:%d, 连接ID:%llu, ip:%s, port:%d\n", type, conn_id, ip.c_str(), port);
    };
    void OnAcceptting(ToolBox::NetworkType type, int32_t fd) override
    {
        Print("[TestUdpNetworkForward] 正准备将新连接加入io多路复用,网络类型:%d fd:%d\n", type, fd);
    };
    void OnAccepted(ToolBox::NetworkType type, uint64_t conn_id) override
    {
        Print("[TestUdpNetworkForward] 接到客户端连接,连接ID:%llu\n", conn_id);
    };
    void OnConnected(ToolBox::NetworkType type, uint64_t conn_id) override
    {
        Print("[TestUdpNetworkForward] 主动连接成功:%llu\n", conn_id);
        echo_conn_id_ = conn_id;
    };
    void OnConnectedFailed(ToolBox::NetworkType type, ToolBox::ENetErrCode err_code, int32_t err_no) override
    {
        Print("[TestUdpNetworkForward] 连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
    };
    void OnErrored(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) override
    {
        Print("[TestUdpNetworkForward] 发生错误, connect_id:%lu 错误码:%d, 系统错误码:%d\n", conn_id,  err_code, err_no);
    }
    void OnReceived(ToolBox::NetworkType type, uint64_t conn_id, const char* data, size_t size) override
    {
        //Print("收到客户端数据长度为%d\n", size);
        //PrintData(data, 32);
        if (conn_id == echo_conn_id_)
        {
            // echo 发来的信息
            // 换头后发送给 client
            //  Print("收到 echo 数据长度为%d\n", size);
            //  PrintData(data, 32);
            uint64_t connect_id = 0;
            memmove(&connect_id, data + sizeof(uint32_t), sizeof(connect_id));

            char* send_data = ToolBox::MemPoolLockFreeMgr->GetMemory(size - sizeof(uint64_t));
            uint32_t send_data_size = size - sizeof(uint64_t);
            uint64_t client_conn_id = 0;
            memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
            memmove(&client_conn_id, data + sizeof(uint32_t), sizeof(uint64_t));
            memmove(send_data + sizeof(uint32_t), data + sizeof(uint32_t) + sizeof(uint64_t), size - sizeof(uint32_t) - sizeof(uint64_t)); // data

            // Print("转发给 client 的数据长度为%d,conn_id:%lu\n", send_data_size, client_conn_id);
            // PrintData(send_data, 32);
            // Print("\n\n");
            Send(ToolBox::NT_UDP, client_conn_id, send_data, send_data_size);

            ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data1");
        }
        else
        {
            // 客户端发来的消息
            // 换头后发送给 echo
            //  Print("收到 client 数据长度为%d\n", size);
            //  PrintData(data, 32);

            char* send_data = ToolBox::MemPoolLockFreeMgr->GetMemory(size + sizeof(uint64_t));
            uint32_t send_data_size = size + sizeof(uint64_t);
            memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
            memmove(send_data + sizeof(uint32_t), &conn_id, sizeof(uint64_t));  // conn_id
            memmove(send_data + sizeof(uint32_t) + sizeof(uint64_t), data + sizeof(uint32_t), size - sizeof(uint32_t)); // data
            // Print("转发给 echo 的数据长度为%d,conn_id:%lu\n", send_data_size, echo_conn_id_);
            //PrintData(send_data, 16);
            Send(ToolBox::NT_UDP, echo_conn_id_, send_data, send_data_size);
            ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data2");
        }

    };
    void OnClose(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err) override
    {
        Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
    }
private:
    uint64_t echo_conn_id_ = -1;
};



FIXTURE_BEGIN(UdpEpollNetwork)

CASE(test_udp_echo)
{
    return;
#ifdef USE_GPERF_TOOLS
    ProfilerStart("test_udp_echo.prof");
#endif // USE_GPERF_TOOLS
    fprintf(stderr, "网络库测试用例: test_udp_echo \n");
    ToolBox::Singleton<TestUdpNetworkEcho>::Instance()->SetDebugPrint(true);
    LogMgr->SetLogLevel(ToolBox::LogLevel::LOG_TRACE);
    ToolBox::Singleton<TestUdpNetworkEcho>::Instance()->Start(4);
    ToolBox::Singleton<TestUdpNetworkEcho>::Instance()->Accept("0.0.0.0", 9600, ToolBox::NT_UDP, 10 * 1024 * 1024, 10 * 1024 * 1024);
    bool run = true;
    std::thread t([&]()
    {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            ToolBox::Singleton<TestUdpNetworkEcho>::Instance()->Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 24 * 3600 * 1000;
    while (true)
    {
        if (used_time > run_mill_seconds)
        {
            break;
        }
        uint32_t time_left = (run_mill_seconds - used_time) / 1000;
        if (time_left != old_time)
        {
            if (time_left + 10 <= old_time || old_time == 0)
            {
                fprintf(stderr, "距离网络库停止还有%d秒\n", time_left);
                old_time = time_left;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    run = false;
    ToolBox::Singleton<TestUdpNetworkEcho>::Instance()->StopWait();
    t.join();
#ifdef USE_GPERF_TOOLS
    ProfilerStop();
#endif // USE_GPERF_TOOLS
    return ;
}


CASE(test_udp_forward)
{
    return;
#ifdef USE_GPERF_TOOLS
    ProfilerStart("test_udp_forward.prof");
#endif // USE_GPERF_TOOLS
    fprintf(stderr, "网络库测试用例: test_udp_forward \n");
    ToolBox::Singleton<TestUdpNetworkForward>::Instance()->SetDebugPrint(true);
    ToolBox::Singleton<TestUdpNetworkForward>::Instance()->Accept("0.0.0.0", 9500, ToolBox::NT_UDP);
    ToolBox::Singleton<TestUdpNetworkForward>::Instance()->Connect("0.0.0.0", 9600, ToolBox::NT_UDP, 10 * 1024 * 1024, 10 * 1024 * 1024);
    ToolBox::Singleton<TestUdpNetworkForward>::Instance()->Start();
    bool run = true;
    std::thread t([&]()
    {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            ToolBox::Singleton<TestUdpNetworkForward>::Instance()->Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 24 * 3600 * 1000;
    while (true)
    {
        if (used_time > run_mill_seconds)
        {
            break;
        }
        uint32_t time_left = (run_mill_seconds - used_time) / 1000;
        if (time_left != old_time)
        {
            if (time_left + 10 <= old_time || old_time == 0)
            {
                fprintf(stderr, "距离网络库停止还有 %d 秒. \n", time_left);
                old_time = time_left;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    run = false;
    t.join();
    ToolBox::Singleton<TestUdpNetworkForward>::Instance()->StopWait();
#ifdef USE_GPERF_TOOLS
    ProfilerStop();
#endif // USE_GPERF_TOOLS
    return;
}

FIXTURE_END(UdpEpollNetwork)

