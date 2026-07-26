#include "udp_epoll_network.h"

#ifdef __linux__

#include "epoll_define.h"
#include "network/net_imp/udp_socket.h"
#include "epoll_ctrl.h"
#include "network/net_imp/socket_pool.h"
#include <vector>

namespace ToolBox
{

    bool UdpEpollNetwork::Init(NetworkChannel* master, NetworkType network_type, uint32_t net_thread_index)
    {
        base_ctrl_ = new EpollCtrl(MAX_SOCKET_COUNT);
        if (!ImpNetwork<UdpSocket>::Init(master, network_type, net_thread_index))
        {
            NetworkLogError("[Network] Init UdpEpollNetwork failed. network_type:%d", network_type);
            return false;
        }
        return true;
    }

    void UdpEpollNetwork::Update(std::time_t time_stamp)
    {
        ImpNetwork<UdpSocket>::Update(time_stamp);
        if (is_kcp_open_)
        {
            // KcpUpdate 可能触发 Close 并删除映射,先复制连接 ID 防迭代器失效
            std::vector<uint32_t> conn_ids;
            conn_ids.reserve(address_to_connect_.size());
            for (const auto& address_conn : address_to_connect_)
            {
                conn_ids.emplace_back(address_conn.second);
            }
            for (uint32_t conn_id : conn_ids)
            {
                auto* socket = sock_mgr_.GetSocket(conn_id);
                if (nullptr == socket)
                {
                    continue;
                }
                socket->KcpUpdate(time_stamp);
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

    void UdpEpollNetwork::OnSocketClosing(UdpSocket* socket, ENetErrCode net_err, int32_t sys_err)
    {
        if (nullptr == socket)
        {
            return;
        }
        if (UdpType::ACCEPTOR != socket->GetType())
        {
            address_to_connect_.erase(socket->GetRemoteAddressID());
            return;
        }

        const int32_t shared_fd = socket->GetSocketID();
        std::vector<uint32_t> remote_conn_ids;
        for (const auto& address_conn : address_to_connect_)
        {
            UdpSocket* remote = sock_mgr_.GetSocket(address_conn.second);
            if (nullptr != remote && remote != socket
                && UdpType::REMOTE == remote->GetType()
                && shared_fd == remote->GetSocketID())
            {
                remote_conn_ids.emplace_back(remote->GetConnID());
            }
        }
        for (uint32_t conn_id : remote_conn_ids)
        {
            UdpSocket* remote = sock_mgr_.GetSocket(conn_id);
            if (nullptr != remote && UdpType::REMOTE == remote->GetType())
            {
                remote->Close(net_err, sys_err);
            }
        }
        address_to_connect_.erase(socket->GetLocalAddressID());
        NetworkLogInfo("[Network] close UDP accepter and dependent remotes. fd:%d remote_count:%zu",
                       shared_fd, remote_conn_ids.size());
    }

    void UdpEpollNetwork::OpenKcpMode()
    {
        is_kcp_open_ = true;
    }


    uint64_t UdpEpollNetwork::OnNewAccepter(uint64_t opaque, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto conn_id = ImpNetwork<UdpSocket>::OnNewAccepter(opaque, ip, port, send_buff_size, recv_buff_size);
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
    uint64_t UdpEpollNetwork::OnNewConnecter(uint64_t opaque, const std::string& ip, const uint16_t port, int32_t send_buff_size, int32_t recv_buff_size)
    {
        auto conn_id = ImpNetwork<UdpSocket>::OnNewConnecter(opaque, ip, port, send_buff_size, recv_buff_size);
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
            NetworkLogDebug("[Network][UdpEpollNetwork] OnConnected. remote_address_id:%llu, conn_id:%llu", remote_address_id, new_socket->GetConnID());
            if (remote_address_id > 0)
            {
                OnConnected(opaque, remote_address_id);
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
        const uint32_t conn_id = iter->second;
        UdpSocket* socket = sock_mgr_.GetSocket(conn_id);
        if (nullptr == socket)
        {
            NetworkLogWarn("[Network] remove stale UDP address mapping. address_id:%llu conn_id:%u",
                           address_id, conn_id);
            address_to_connect_.erase(iter);
            return;
        }
        ImpNetwork<UdpSocket>::OnClose(conn_id);
    }

    void UdpEpollNetwork::OnSend(uint64_t address_id, const char* data, uint32_t size)
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