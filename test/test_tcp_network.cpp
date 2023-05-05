#include "network/network_api.h"
#include "unit_test_frame/unittest.h"
#include "tools/log.h"
#include <stdint.h>
#include "tools/time_util.h"
#include "tools/memory_pool_lock_free.h"
#ifdef USE_GPERF_TOOLS
#include <gperftools/profiler.h>
#endif // USE_GPERF_TOOLS
// class TestNetworkEcho : public ToolBox::NetworkChannel, public ToolBox::DebugPrint
// {
// public:
//     void OnBinded(ToolBox::NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port) override
//     {
//         Print("[TestNetworkEcho] 服务器已建立监听端口,网络类型:%d, 连接ID:%llu, ip:%s, port:%d\n", type, conn_id, ip.c_str(), port);
//     };
//     void OnAccepting(ToolBox::NetworkType type, int32_t fd) override
//     {
//         Print("[TestNetworkEcho] 正准备将新连接加入io多路复用,网络类型:%d fd:%d\n", type, fd);
//     };
//     void OnAccepted(ToolBox::NetworkType type, uint64_t conn_id) override
//     {
//         Print("[TestNetworkEcho] 接到客户端连接,连接ID:%llu\n", conn_id);
//     };
//     void OnConnected(ToolBox::NetworkType type, uint64_t conn_id) override
//     {
//         Print("[TestNetworkEcho] 主动连接成功:%llu\n", conn_id);
//     };
//     void OnConnectedFailed(ToolBox::NetworkType type, ToolBox::ENetErrCode err_code, int32_t err_no) override
//     {
//         Print("[TestNetworkEcho] 连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
//     };
//     void OnErrored(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) override
//     {
//         Print("[TestNetworkEcho] 发生错误, connect_id: %lu 错误码:%d, 系统错误码:%d.\n", conn_id,  err_code, err_no);
//     }
//     void OnReceived(ToolBox::NetworkType type, uint64_t conn_id, const char* data, size_t size) override
//     {
//         // Print("收到客户端数据长度为%d,conn_id:%lu\n", size, conn_id);
//         // PrintData(data, 16);
//         Send(ToolBox::NT_TCP, conn_id, data, size);

//         recv_packets_++;
//         std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
//         if (now_time > last_update_time_ + 1000)
//         {
//             Print("[TestNetworkEcho] 连接ID:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
//             last_update_time_ = now_time;
//             recv_packets_ = 0;
//         }
//         uint64_t connect_id = 0;
//         memmove(&connect_id, data + sizeof(uint32_t), sizeof(connect_id));
//         if (3 == connect_id)
//         {
//             // Print("[TestNetworkEcho] 连接ID:%llu now_time:%llu, data size:%zu\n", connect_id, now_time, size);
//         }
//     };
//     void OnClose(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err) override
//     {
//         Print("[TestNetworkEcho] 断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
//     }
// private:
//     std::time_t last_update_time_ = 0;
//     uint32_t recv_packets_ = 0;
// };

// class TestNetworkForward : public ToolBox::NetworkChannel, public ToolBox::DebugPrint
// {
// public:
//     void OnBinded(ToolBox::NetworkType type, uint64_t conn_id, const std::string& ip, uint16_t port) override
//     {
//         Print("[TestNetworkForward] 服务器已建立监听端口,网络类型:%d, 连接ID:%llu, ip:%s, port:%d\n", type, conn_id, ip.c_str(), port);
//     };
//     void OnAccepting(ToolBox::NetworkType type, int32_t fd) override
//     {
//         Print("[TestNetworkForward] 正准备将新连接加入io多路复用,网络类型:%d fd:%d\n", type, fd);
//     };
//     void OnAccepted(ToolBox::NetworkType type, uint64_t conn_id) override
//     {
//         Print("[TestNetworkForward] 接到客户端连接,连接ID:%llu\n", conn_id);
//     };
//     void OnConnected(ToolBox::NetworkType type, uint64_t conn_id) override
//     {
//         Print("[TestNetworkForward] 主动连接成功:%llu\n", conn_id);
//         echo_conn_id_ = conn_id;
//     };
//     void OnConnectedFailed(ToolBox::NetworkType type, ToolBox::ENetErrCode err_code, int32_t err_no) override
//     {
//         Print("[TestNetworkForward] 连接失败 错误码:%d, 系统错误码:%d\n",  err_code, err_no);
//     };
//     void OnErrored(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no) override
//     {
//         Print("[TestNetworkForward] 发生错误, connect_id:%lu 错误码:%d, 系统错误码:%d\n", conn_id,  err_code, err_no);
//     }
//     void OnReceived(ToolBox::NetworkType type, uint64_t conn_id, const char* data, size_t size) override
//     {
//         //Print("收到客户端数据长度为%d\n", size);
//         //PrintData(data, 32);
//         if (conn_id == echo_conn_id_)
//         {
//             // echo 发来的信息
//             // 换头后发送给 client
//             //   Print("收到 echo 数据长度为%d\n", size);
//             //   PrintData(data, 32);
//             uint64_t connect_id = 0;
//             memmove(&connect_id, data + sizeof(uint32_t), sizeof(connect_id));

//             char* send_data = ToolBox::MemPoolLockFreeMgr->GetMemory(size - sizeof(uint64_t));
//             uint32_t send_data_size = size - sizeof(uint64_t);
//             uint64_t client_conn_id = 0;
//             memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
//             memmove(&client_conn_id, data + sizeof(uint32_t), sizeof(uint64_t));
//             memmove(send_data + sizeof(uint32_t), data + sizeof(uint32_t) + sizeof(uint64_t), size - sizeof(uint32_t) - sizeof(uint64_t)); // data

//             // Print("转发给 client 的数据长度为%d,conn_id:%lu\n", send_data_size, client_conn_id);
//             // PrintData(send_data, 32);
//             // Print("\n\n");
//             Send(ToolBox::NT_TCP, client_conn_id, send_data, send_data_size);

//             ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data1");


//             std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
//             recv_packets_++;
//             if (now_time > last_update_time_ + 1000)
//             {
//                 Print("[TestNetworkForward] 连接ID:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
//                 last_update_time_ = now_time;
//                 recv_packets_ = 0;
//             }
//             if (3 == client_conn_id)
//             {
//                 // Print("[TestNetworkForward] 连接ID:%llu from echo now_time:%llu, data size:%zu\n", client_conn_id, now_time, size);
//             }
//         }
//         else
//         {
//             // 客户端发来的消息
//             // 换头后发送给 echo
//             //   Print("收到 client 数据长度为%d\n", size);
//             //   PrintData(data, 32);

//             char* send_data = ToolBox::MemPoolLockFreeMgr->GetMemory(size + sizeof(uint64_t));
//             uint32_t send_data_size = size + sizeof(uint64_t);
//             memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
//             memmove(send_data + sizeof(uint32_t), &conn_id, sizeof(uint64_t));  // conn_id
//             memmove(send_data + sizeof(uint32_t) + sizeof(uint64_t), data + sizeof(uint32_t), size - sizeof(uint32_t)); // data
//             // Print("转发给 echo 的数据长度为%d,conn_id:%lu\n", send_data_size, echo_conn_id_);
//             //PrintData(send_data, 16);
//             Send(ToolBox::NT_TCP, echo_conn_id_, send_data, send_data_size);
//             ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data2");

//             client_recv_packets_++;
//             std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
//             if (now_time > client_last_update_time_ + 1000)
//             {
//                 Print("[TestNetworkForward] 客户端:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
//                 client_last_update_time_ = now_time;
//                 client_recv_packets_ = 0;
//             }
//             if (3 == conn_id)
//             {
//                 // Print("[TestNetworkForward] 连接ID:%llu from client now_time:%llu\n", conn_id, now_time);
//             }
//         }

//     };
//     void OnClose(ToolBox::NetworkType type, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err) override
//     {
//         Print("断开与客户端之间的连接,连接ID:%llu 错误码:%d, 系统错误码:%d\n", conn_id, net_err, sys_err);
//     }
// private:
//     uint64_t echo_conn_id_ = -1;
// private:
//     std::time_t last_update_time_ = 0;
//     uint32_t recv_packets_ = 0;

//     std::time_t client_last_update_time_ = 0;
//     uint32_t client_recv_packets_ = 0;
// };

FIXTURE_BEGIN(TcpNetwork)

CASE(test_tcp_echo)
{
    return;
#ifdef USE_GPERF_TOOLS
    ProfilerStart("test_tcp_echo.prof");
#endif // USE_GPERF_TOOLS
    fprintf(stderr, "网络库测试用例: test_tcp_echo \n");
    std::time_t last_update_time_ = 0;
    uint32_t recv_packets_ = 0;
    LogMgr->SetLogLevel(ToolBox::LogLevel::LOG_TRACE);
    ToolBox::Network network_echo;
    network_echo.SetOnBinded([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const std::string & ip, uint16_t port)
    {
        printf("[TestNetworkEcho] 服务器已建立监听端口,网络类型:%d,连接标记:%llu, 连接ID:%llu, ip:%s, port:%d\n", type, opaque, conn_id, ip.c_str(), port);
    }).SetOnAccepting([](ToolBox::NetworkType type, uint64_t opaque, int32_t fd)
    {
        printf("[TestNetworkEcho] 正准备将新连接加入io多路复用,网络类型:%d,连接标记:%llu, fd:%d\n", type, opaque, fd);
    }).SetOnAccepted([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id)
    {
        printf("[TestNetworkEcho] 接到客户端连接,网络类型:%d,连接标记:%llu,连接ID:%llu\n", type, opaque, conn_id);
    }).SetOnConnected([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id)
    {
        printf("[TestNetworkEcho] 主动连接成功:网络类型:%d,连接标记:%llu,连接ID:%llu\n", type, opaque, conn_id);
    }).SetOnConnectFailed([](ToolBox::NetworkType type, uint64_t opaque, ToolBox::ENetErrCode err_code, int32_t err_no)
    {
        printf("[TestNetworkEcho] 连接失败.网络类型:%d,连接标记:%llu, 错误码:%d, 系统错误码:%d\n", type, opaque, err_code, err_no);
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no)
    {
        printf("[TestNetworkEcho] 发生错误, 网络类型:%d,连接标记:%llu, connect_id: %lu 错误码:%d, 系统错误码:%d.\n", type, opaque, conn_id,  err_code, err_no);
    }).SetOnReceived([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const char* data, size_t size)
    {
        // printf("收到客户端数据长度为%d,conn_id:%lu\n", size, conn_id);
        // PrintData(data, 16);
        network_echo.Send(conn_id, data, size);

        recv_packets_++;
        std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
        if (now_time > last_update_time_ + 1000)
        {
            printf("[TestNetworkEcho] 连接ID:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
            last_update_time_ = now_time;
            recv_packets_ = 0;
        }
        uint64_t connect_id = 0;
        memmove(&connect_id, data + sizeof(uint32_t), sizeof(connect_id));
        if (3 == connect_id)
        {
            // printf("[TestNetworkEcho] 连接ID:%llu now_time:%llu, data size:%zu\n", connect_id, now_time, size);
        }
    }).SetOnClose([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err)
    {
        printf("[TestNetworkEcho] 断开与客户端之间的连接,网络类型:%d,连接标记:%llu,连接ID:%llu 错误码:%d, 系统错误码:%d\n", type, opaque, conn_id, net_err, sys_err);
    });
    network_echo.Start(1);
    network_echo.SetSimulateNagle(256, 1);
    network_echo.Accept(ToolBox::NT_TCP, 9600, "0.0.0.0", 9600,  10 * 1024 * 1024, 10 * 1024 * 1024);
    bool run = true;
    std::thread t([&]()
    {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            network_echo.Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 30000 * 1000;
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
                fprintf(stderr, "距离网络库停止还有%d秒 \n", time_left);
                old_time = time_left;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        used_time += 100;
    }

    run = false;
    t.join();
    network_echo.StopWait();
#ifdef USE_GPERF_TOOLS
    ProfilerStop();
#endif // USE_GPERF_TOOLS
    return ;
}

CASE(test_tcp_forward)
{
    // return;
#ifdef USE_GPERF_TOOLS
    ProfilerStart("test_tcp_forward.prof");
#endif // USE_GPERF_TOOLS
    fprintf(stderr, "网络库测试用例: test_tcp_forward \n");

    uint64_t echo_conn_id_ = -1;
    std::time_t last_update_time_ = 0;
    uint32_t recv_packets_ = 0;
    std::time_t client_last_update_time_ = 0;
    uint32_t client_recv_packets_ = 0;

    LogMgr->SetLogLevel(ToolBox::LogLevel::LOG_TRACE);
    ToolBox::Network network_forward;
    network_forward.SetOnBinded([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const std::string & ip, uint16_t port)
    {
        printf("[TestNetworkForward] 服务器已建立监听端口, 网络类型:%d,连接标记:%llu, 连接ID:%llu, ip:%s, port:%d\n", type, opaque, conn_id, ip.c_str(), port);
    }).SetOnAccepting([](ToolBox::NetworkType type, uint64_t opaque, int32_t fd)
    {
        printf("[TestNetworkForward] 正准备将新连接加入io多路复用, 网络类型:%d, 连接标记:%llu, fd:%d\n", type, opaque, fd);
    }).SetOnAccepted([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id)
    {
        printf("[TestNetworkForward] 接到客户端连接, 网络类型:%d, 连接标记:%llu, 连接ID:%llu\n", type, opaque, conn_id);
    }).SetOnConnected([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id)
    {
        echo_conn_id_ = conn_id;
        printf("[TestNetworkForward] 主动连接成功.网络类型:%d, 连接标记:%llu, 连接ID:%llu\n", type, opaque, conn_id);
    }).SetOnConnectFailed([](ToolBox::NetworkType type, uint64_t opaque, ToolBox::ENetErrCode err_code, int32_t err_no)
    {
        printf("[TestNetworkForward] 连接失败.网络类型:%d, 连接标记:%llu, 错误码:%d, 系统错误码:%d\n", type, opaque, err_code, err_no);
    }).SetOnErrored([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode err_code, int32_t err_no)
    {
        printf("[TestNetworkForward] 发生错误, 网络类型:%d, 连接标记:%llu, connect_id: %lu 错误码:%d, 系统错误码:%d.\n", type, opaque, conn_id,  err_code, err_no);
    }).SetOnReceived([&](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, const char* data, size_t size)
    {
        //Print("收到客户端数据长度为%d\n", size);
        //PrintData(data, 32);
        if (conn_id == echo_conn_id_)
        {
            // echo 发来的信息
            // 换头后发送给 client
            //   Print("收到 echo 数据长度为%d\n", size);
            //   PrintData(data, 32);
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
            network_forward.Send(client_conn_id, send_data, send_data_size);

            ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data1");


            std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
            recv_packets_++;
            if (now_time > last_update_time_ + 1000)
            {
                printf("[TestNetworkForward] 连接ID:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
                last_update_time_ = now_time;
                recv_packets_ = 0;
            }
            if (3 == client_conn_id)
            {
                // printf("[TestNetworkForward] 连接ID:%llu from echo now_time:%llu, data size:%zu\n", client_conn_id, now_time, size);
            }
        }
        else
        {
            // 客户端发来的消息
            // 换头后发送给 echo
            //   printf("收到 client 数据长度为%d\n", size);
            //   PrintData(data, 32);

            char* send_data = ToolBox::MemPoolLockFreeMgr->GetMemory(size + sizeof(uint64_t));
            uint32_t send_data_size = size + sizeof(uint64_t);
            memmove(send_data, &send_data_size, sizeof(uint32_t));  // buff len
            memmove(send_data + sizeof(uint32_t), &conn_id, sizeof(uint64_t));  // conn_id
            memmove(send_data + sizeof(uint32_t) + sizeof(uint64_t), data + sizeof(uint32_t), size - sizeof(uint32_t)); // data
            // printf("转发给 echo 的数据长度为%d,conn_id:%lu\n", send_data_size, echo_conn_id_);
            //PrintData(send_data, 16);
            network_forward.Send(echo_conn_id_, send_data, send_data_size);
            ToolBox::MemPoolLockFreeMgr->GiveBack(send_data, "test_send_data2");

            client_recv_packets_++;
            std::time_t now_time = ToolBox::GetMillSecondTimeStamp();
            if (now_time > client_last_update_time_ + 1000)
            {
                printf("[TestNetworkForward] 客户端:%llu 每秒收到了 %d 个数据包\n", conn_id, recv_packets_);
                client_last_update_time_ = now_time;
                client_recv_packets_ = 0;
            }
            if (3 == conn_id)
            {
                // Print("[TestNetworkForward] 连接ID:%llu from client now_time:%llu\n", conn_id, now_time);
            }
        }
    }).SetOnClose([](ToolBox::NetworkType type, uint64_t opaque, uint64_t conn_id, ToolBox::ENetErrCode net_err, int32_t sys_err)
    {
        printf("[TestNetworkForward] 断开与客户端之间的连接, 网络类型:%d, 连接标记:%llu, 连接ID:%llu 错误码:%d, 系统错误码:%d\n", type, opaque, conn_id, net_err, sys_err);
    });
    network_forward.Start(2);
    network_forward.Accept(ToolBox::NT_TCP, 9500, "0.0.0.0", 9500);
    network_forward.Connect(ToolBox::NT_TCP, 9600, "0.0.0.0", 9600,  10 * 1024 * 1024, 10 * 1024 * 1024);
    bool run = true;
    std::thread t([&]()
    {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            network_forward.Update();
        }
    });
    uint32_t used_time = 0;
    uint32_t old_time = 0;
    uint32_t run_mill_seconds = 3600 * 1000;
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
    network_forward.StopWait();
#ifdef USE_GPERF_TOOLS
    ProfilerStop();
#endif // USE_GPERF_TOOLS
    return;
}

FIXTURE_END(TcpNetwork)