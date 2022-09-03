#pragma once
#ifdef __linux__
#include "network/net_imp/imp_network.h"
#include <unordered_map>

namespace ToolBox
{

    class UdpSocket;
    class UdpAddress;
    /*
    * 定义基于 UDP 和 Epoll 的网络
    */
    class UdpEpollNetwork : public ImpNetwork<UdpSocket>
    {
    public:
        /*
        * 构造
        */
        UdpEpollNetwork() = default;
        /*
        * 析构
        */
        virtual ~UdpEpollNetwork() = default;
        /*
        * 初始化
        */
        virtual bool Init(NetworkChannel* master, NetworkType network_type) override;
        /*
        * 执行一次网络循环
        */
        virtual void Update() override;
    public:
        /*
        * @brief UdpAddress 是否存在
        * @param udp_address 远端udp地址
        * @return != nullptr:udpsocket指针; nullptr:找不到socket
        */
        UdpSocket* GetSocketByUdpAddress(const UdpAddress& udp_address);
        /*
        * @brief 增加地址映射
        * @param udp_address udp地址
        */
        void AddUdpAddress(const UdpAddress& udp_address, uint32_t conn_id);
        /*
        * @brief 删除地址映射
        * @param udp_address udp地址
        */
        void DeleteUdpAddress(const UdpAddress& udp_address);
        /*
        * @brief 开启Kcp模式
        */
        void OpenKcpMode();
        /*
        * @brief kcp 模式是否开启
        * @return bool true:kcp false:udp
        */
        bool IsKcpModeOpen()
        {
            return is_kcp_open_;
        };
    protected:
        /*
        * 工作线程内建立监听器
        */
        virtual uint64_t OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
        /*
        * 工作线程内建立连接器
        */
        virtual uint64_t OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size) override;
        /*
        * 工作线程内闭网络连接
        */
        virtual void OnClose(uint64_t address_id) override;
        /*
        * 工作线程内工作线程内发送
        */
        virtual void OnSend(uint64_t address_id, const char* data, std::size_t size) override;

    private:
        std::unordered_map<uint64_t, uint32_t> address_to_connect_;      // 地址转换的ID 到 SocketPool管理的连接ID的映射
        bool is_kcp_open_ = false;      // KCP是否开启
    };

};  // ToolBox

#endif // __linux__