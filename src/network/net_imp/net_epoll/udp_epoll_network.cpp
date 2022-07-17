#include "udp_epoll_network.h"

#ifdef __linux__

#include "epoll_define.h"
#include "network/net_imp/udp_socket.h"
#include "tools/time_util.h"
#include "epoll_ctrl.h"
#include "network/net_imp/socket_pool.h"

namespace ToolBox
{

    bool UdpEpollNetwork::Init(NetworkMaster* master, NetworkType network_type)
    {
        base_ctrl_ = new EpollCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<UdpSocket>::Init(master, network_type))
        {
            NetworkLogError("[Network] Init UdpEpollNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

    void UdpEpollNetwork::Update()
    {
        ImpNetwork<UdpSocket>::Update();
        if (is_kcp_open_)
        {
            auto current = GetMillSecondTimeStamp();
            // ���� kcp [TODO: �����update�����Ż��ռ�]
            for (auto iter : address_to_connect_)
            {
                auto socket = sock_mgr_.GetSocket(iter.second);
                if (nullptr == socket)
                {
                    continue;
                }
                socket->KcpUpdate(current);
            }
        }
    }

    UdpSocket* UdpEpollNetwork::GetSocketByUdpAddress(const UdpAddress& udp_address)
    {
        auto iter = address_to_connect_.find(udp_address.GetID());
        if (iter != address_to_connect_.end())
        {
            return sock_mgr_.GetSocket(iter->second);
        }
        else
        {
            return nullptr;
        }
    }

    void UdpEpollNetwork::AddUdpAddress(const UdpAddress& udp_address, uint32_t conn_id)
    {
        address_to_connect_[udp_address.GetID()] = conn_id;
    }
    void UdpEpollNetwork::DeleteUdpAddress(const UdpAddress& udp_address)
    {
        address_to_connect_.erase(udp_address.GetID());
    }

    void UdpEpollNetwork::OpenKcpMode()
    {
        is_kcp_open_ = true;
    }


    uint64_t UdpEpollNetwork::OnNewAccepter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto conn_id = ImpNetwork<UdpSocket>::OnNewAccepter(ip, port, send_buff_size, recv_buff_size);
        auto* new_socket = sock_mgr_.GetSocket(conn_id);
        if (nullptr != new_socket)
        {
            // KCP
            if (is_kcp_open_)
            {
                new_socket->OpenKcpMode();
            }
            address_to_connect_[new_socket->GetLocalAddressID()] = new_socket->GetConnID();
            return new_socket->GetLocalAddressID();
        }
        return INVALID_CONN_ID;
    }
    uint64_t UdpEpollNetwork::OnNewConnecter(const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto conn_id = ImpNetwork<UdpSocket>::OnNewConnecter(ip, port, send_buff_size, recv_buff_size);
        auto* new_socket = sock_mgr_.GetSocket(conn_id);
        if (nullptr != new_socket)
        {
            // KCP
            if (is_kcp_open_)
            {
                new_socket->OpenKcpMode();
            }
            uint64_t remote_address_id = new_socket->GetRemoteAddressID();
            address_to_connect_[remote_address_id] = new_socket->GetConnID();
            if (remote_address_id > 0)
            {
                OnConnected(remote_address_id);
            }
            return remote_address_id;
        }
        return INVALID_CONN_ID;
    }
    void UdpEpollNetwork::OnClose(uint64_t address_id)
    {
        auto iter = address_to_connect_.find(address_id);
        if (iter == address_to_connect_.end())
        {
            return;
        }
        ImpNetwork<UdpSocket>::OnClose(iter->second);
        address_to_connect_.erase(iter);
    }

    void UdpEpollNetwork::OnSend(uint64_t address_id, const char* data, std::size_t size)
    {
        auto iter = address_to_connect_.find(address_id);
        if (iter == address_to_connect_.end())
        {
            return;
        }
        auto socket = sock_mgr_.GetSocket(iter->second);
        if (nullptr == socket)
        {
            return;
        }
        // KCP
        if (is_kcp_open_)
        {
            socket->KcpSendTo(data, size);

        }
        else
        {
            socket->Send(data, size);
        }
    }

};  // ToolBox

#endif // __linux__