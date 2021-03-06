#include "src/network/network_mgr.h"
#include "unit_test_frame/unittest.h"

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
    void OnReceived(uint64_t conn_id, const char* data, size_t size) override
    {
        Print("收到客户端数据长度为%d\n", size);
        PrintData(data, 32);
        Send(NT_TCP, conn_id, data, size);
    };
    void OnClose(uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) override
    {
        Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
    }
};

class TestNetworkForward : public NetworkMaster, public DebugPrint
{
public:
    void OnAccepted(uint64_t conn_id) override 
    {
        Print("接到客户端连接,连接ID:%llu\n", conn_id);
    };
    void OnConnected(uint64_t conn_id) override
    {
        Print("主动连接成功:%llu\n", conn_id);
        echo_conn_id_ = conn_id;
    };
    void OnConnectedFailed(ENetErrCode err_code, int32_t err_no) override
    {
        Print("连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
    };
    void OnReceived(uint64_t conn_id, const char* data, size_t size) override
    {
        //Print("收到客户端数据长度为%d\n", size);
        //PrintData(data, 32);
        if(conn_id == echo_conn_id_)
        {
            // echo 发来的信息
            // 换头后发送给 client
            Print("收到 echo 数据长度为%d\n", size);
            PrintData(data, 32);
            uint64_t connect_id = 0;
            memmove(&connect_id, data + sizeof(uint32_t), sizeof(connect_id));

            char* send_data = MemPoolMgr->GetMemory(size - sizeof(uint64_t));
            uint32_t send_data_size = size - sizeof(uint64_t);  
            memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
            memmove(send_data + sizeof(uint32_t), data + sizeof(uint32_t) + sizeof(uint64_t), size - sizeof(uint32_t) -  + sizeof(uint64_t)); // data

            Print("转发给 client 的数据长度为%d\n", send_data_size);
            PrintData(data, 32);
            Send(NT_TCP, echo_conn_id_, send_data, send_data_size);
            
            MemPoolMgr->GiveBack(send_data);
        } else 
        {
            // 客户端发来的消息
            // 换头后发送给 echo
            Print("收到 client 数据长度为%d\n", size);
            PrintData(data, 32);

            char* send_data = MemPoolMgr->GetMemory(size + sizeof(uint64_t));
            uint32_t send_data_size = size + sizeof(uint64_t);  
            Print("send_data_size: %u\n", send_data_size);
            memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
            PrintData(send_data, 32);
            memmove(send_data + sizeof(uint32_t), &conn_id, sizeof(uint64_t));  // conn_id
            PrintData(send_data, 32);
            memmove(send_data + sizeof(uint32_t) + sizeof(uint64_t), data + sizeof(uint32_t), size - sizeof(uint32_t)); // data

            Print("转发给 echo 的数据长度为%d\n", send_data_size);
            PrintData(send_data, 32);

            Send(NT_TCP, echo_conn_id_, send_data, size);

            MemPoolMgr->GiveBack(send_data);
        }

    };
    void OnClose(uint64_t conn_id, ENetErrCode net_err, int32_t sys_err) override
    {
        Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
    }
private:
    uint64_t echo_conn_id_ = 0;
};

FIXTURE_BEGIN(TCPNetWork)

CASE(test_tcp_echo)
{
    return;
    fprintf(stderr,"网络库测试用例: test_tcp_echo \n");
    Singleton<TestNetworkEcho>::Instance()->SetDebugPrint(true);
    Singleton<TestNetworkEcho>::Instance()->Accept("127.0.0.1", 9600, NT_TCP);
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
    uint32_t run_mill_seconds = 1000*1000;
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
    t.join();
    Singleton<NetworkMaster>::Instance()->StopWait();
    return ;
}

CASE(test_tcp_forward)
{
    // return;
    fprintf(stderr,"网络库测试用例: test_tcp_forward \n");
    Singleton<TestNetworkForward>::Instance()->SetDebugPrint(true);
    Singleton<TestNetworkForward>::Instance()->Accept("127.0.0.1", 9500, NT_TCP);
    Singleton<TestNetworkForward>::Instance()->Connect("127.0.0.1", 9600, NT_TCP);
    Singleton<TestNetworkForward>::Instance()->Start();
    bool run = true;
    std::thread t([&](){
        while(run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Singleton<TestNetworkForward>::Instance()->Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 1000*1000;
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
    t.join();
    Singleton<NetworkMaster>::Instance()->StopWait();
    return;
}

FIXTURE_END(TCPNetWork)