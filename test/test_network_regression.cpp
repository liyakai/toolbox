/*
 * 网络库综合回归测试(单进程 loopback,无需多进程).
 *
 * 设计约定:
 * 1. 每个 CASE 独占自己的 Network 实例,并用 NetworkStopGuard 保证退出路径必定 StopWait,
 *    避免断言失败或提前 return 时遗留网络线程污染后续用例.
 * 2. TCP 用单实例自连接;UDP 因 conn_id 由对端地址派生,必须拆成服务端/客户端两个实例
 *    (原因见下方 UDP 段注释).
 * 3. 主线程通过 RequireWait/RequireWaitBoth/RequireWaitAll 轮询 Update 驱动事件派发,
 *    统一超时,不做长跑 sleep;三实例场景须用 RequireWaitAll 同时驱动三侧.
 * 4. Echo 只允许服务端(accepted)连接回包,否则客户端与服务端会形成无限 ping-pong.
 * 5. 所有 Connect 之前必须先等到 Binded,避免监听尚未 bind 完成导致偶发连接失败.
 * 6. NET_ASSERT 直接 abort,保证 Release(NDEBUG)构建下失败不会被静默跳过.
 *
 * 覆盖本轮改动: Start/Stop 幂等与 resume、Start 前缓存 flush、Accepting fd 交割、
 * 回调内 StopWait(Accepting/Accepted/Connected/Received)、Update 重入、多线程路由、
 * TCP 收发/关闭/空包/Nagle(包数刷出+超时刷出)/端口冲突、ConnectFailed(失败无 Closed、不风暴)、
 * UDP 收发/空包整帧投递/多线程(Start2)路由/级联关闭/关 REMOTE 保 ACCEPTOR/CONNECTOR 关闭、
 * KCP 可靠 echo(整帧精确匹配,见 MakeFramed).
 * 未覆盖: event2main_ overflow 全局 FIFO、Start flush 中途失败补 NET_NOT_STARTED
 * (cache 上限远小于 worker 队列,公开 API 难构).io_uring 为实验后端,默认不编.
 */
#include "network/network_api.h"
#include "tools/time_util.h"
#include "unit_test_frame/unittest.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{

constexpr int32_t kDefaultTimeoutMs = 5000;
constexpr int32_t kConnectFailTimeoutMs = 8000;
constexpr int32_t kPollSleepMs = 1;
constexpr const char* kLoopbackIp = "127.0.0.1";
constexpr std::size_t kInvalidIndex = static_cast<std::size_t>(-1);

/*
 * 每个用例占用独立端口,避免同一轮内 TIME_WAIT 或并存监听冲突.
 */
uint16_t NextTestPort()
{
    static std::atomic<uint16_t> port{29100};
    return port.fetch_add(1);
}

// 断言失败直接 abort:不依赖 assert,Release 构建同样生效
#define NET_ASSERT(cond, fmt, ...) \
    do \
    { \
        if (!(cond)) \
        { \
            fprintf(stderr, "[NetworkRegression] ASSERT FAIL %s:%d " fmt "\n", \
                    __FILE__, __LINE__, ##__VA_ARGS__); \
            fflush(stderr); \
            abort(); \
        } \
    } while (0)

enum class ProbeEvent : uint8_t
{
    Binded = 0,
    Accepting,
    Accepted,
    Connected,
    ConnectFailed,
    Errored,
    Closed,
    Received,
    MaxCount,   // 计数数组长度,新增事件只需插在此之前
};

constexpr std::size_t kProbeEventCount = static_cast<std::size_t>(ProbeEvent::MaxCount);

const char* ProbeEventName(ProbeEvent event)
{
    switch (event)
    {
        case ProbeEvent::Binded:
            return "Binded";
        case ProbeEvent::Accepting:
            return "Accepting";
        case ProbeEvent::Accepted:
            return "Accepted";
        case ProbeEvent::Connected:
            return "Connected";
        case ProbeEvent::ConnectFailed:
            return "ConnectFailed";
        case ProbeEvent::Errored:
            return "Errored";
        case ProbeEvent::Closed:
            return "Closed";
        case ProbeEvent::Received:
            return "Received";
        default:
            return "Unknown";
    }
}

/*
 * 回调探针:记录事件顺序、conn_id 与 opaque 的对应关系、每连接收包序列,
 * 并提供 WaitUntil 在主线程驱动 Update.
 */
class NetworkProbe
{
public:
    explicit NetworkProbe(ToolBox::Network& network)
        : network_(network)
    {
        BindCallbacks();
    }

    NetworkProbe(const NetworkProbe&) = delete;
    NetworkProbe& operator=(const NetworkProbe&) = delete;

    // ---------------- 行为开关 ----------------
    /*
     * 仅服务端 accepted 连接回包.客户端不回包,避免无限 ping-pong.
     */
    void EnableServerEcho(bool enable)
    {
        server_echo_ = enable;
    }
    void EnableStopInAccepting(bool enable)
    {
        stop_in_accepting_ = enable;
    }
    void EnableStopInAccepted(bool enable)
    {
        stop_in_accepted_ = enable;
    }
    void EnableStopInConnected(bool enable)
    {
        stop_in_connected_ = enable;
    }
    void EnableStopInReceived(bool enable)
    {
        stop_in_received_ = enable;
    }
    void EnableReenterUpdateInReceived(bool enable)
    {
        reenter_update_in_received_ = enable;
    }

    // ---------------- 驱动与等待 ----------------
    /*
     * 驱动一次主线程派发.多 Network 实例场景需要外部轮流 Pump 每个实例.
     */
    void Pump()
    {
        network_.Update();
    }

    bool WaitUntil(const std::function<bool()>& pred, int32_t timeout_ms = kDefaultTimeoutMs)
    {
        const std::time_t begin = ToolBox::GetMillSecondTimeStamp();
        while (!pred())
        {
            network_.Update();
            if (pred())
            {
                return true;
            }
            if (ToolBox::GetMillSecondTimeStamp() - begin > static_cast<std::time_t>(timeout_ms))
            {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
        }
        return true;
    }

    /*
     * 空转若干轮 Update,用于验证"不应再有新回调"这类否定断言.
     */
    void PumpIdle(int32_t rounds = 20)
    {
        for (int32_t i = 0; i < rounds; ++i)
        {
            network_.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
        }
    }

    void Reset()
    {
        event_seq_.clear();
        counters_.fill(0);
        update_reenter_count_ = 0;
        last_accepting_fd_ = -1;
        last_error_ = ToolBox::ENetErrCode::NET_SUCCESS;
        last_sys_err_ = 0;
        binded_opaques_.clear();
        accepted_opaques_.clear();
        connected_opaques_.clear();
        closed_order_.clear();
        recv_payloads_.clear();
        recv_opaques_.clear();
        accepting_fds_.clear();
        // 行为开关由用例显式管理,Reset 不清,避免 restart 路径误清意图
    }

    // ---------------- 计数与状态 ----------------
    uint32_t Count(ProbeEvent event) const
    {
        return counters_[static_cast<std::size_t>(event)];
    }

    uint32_t TotalCallbackCount() const
    {
        uint32_t total = 0;
        for (uint32_t value : counters_)
        {
            total += value;
        }
        return total;
    }

    /*
     * 事件首次出现的下标,用于断言跨事件的先后顺序.
     */
    std::size_t FirstIndexOf(ProbeEvent event) const
    {
        for (std::size_t idx = 0; idx < event_seq_.size(); ++idx)
        {
            if (event_seq_[idx] == event)
            {
                return idx;
            }
        }
        return kInvalidIndex;
    }

    bool HappensBefore(ProbeEvent first, ProbeEvent second) const
    {
        const std::size_t first_idx = FirstIndexOf(first);
        const std::size_t second_idx = FirstIndexOf(second);
        return kInvalidIndex != first_idx && kInvalidIndex != second_idx && first_idx < second_idx;
    }

    uint32_t update_reenter_count() const
    {
        return update_reenter_count_;
    }
    int32_t last_accepting_fd() const
    {
        return last_accepting_fd_;
    }
    const std::vector<int32_t>& accepting_fds() const
    {
        return accepting_fds_;
    }
    ToolBox::ENetErrCode last_error() const
    {
        return last_error_;
    }
    int32_t last_sys_err() const
    {
        return last_sys_err_;
    }

    bool IsClosed(uint64_t conn_id) const
    {
        for (uint64_t closed : closed_order_)
        {
            if (closed == conn_id)
            {
                return true;
            }
        }
        return false;
    }

    std::size_t ClosedIndexOf(uint64_t conn_id) const
    {
        for (std::size_t idx = 0; idx < closed_order_.size(); ++idx)
        {
            if (closed_order_[idx] == conn_id)
            {
                return idx;
            }
        }
        return kInvalidIndex;
    }

    // ---------------- 连接与收包查询 ----------------
    uint64_t FirstBinded() const
    {
        NET_ASSERT(!binded_opaques_.empty(), "no binded conn");
        return binded_opaques_.begin()->first;
    }
    uint64_t FirstAccepted() const
    {
        NET_ASSERT(!accepted_opaques_.empty(), "no accepted conn");
        return accepted_opaques_.begin()->first;
    }
    uint64_t FirstConnected() const
    {
        NET_ASSERT(!connected_opaques_.empty(), "no connected conn");
        return connected_opaques_.begin()->first;
    }

    uint64_t OpaqueOfBinded(uint64_t conn_id) const
    {
        auto iter = binded_opaques_.find(conn_id);
        NET_ASSERT(iter != binded_opaques_.end(), "binded conn:%lu not found", conn_id);
        return iter->second;
    }
    uint64_t OpaqueOfAccepted(uint64_t conn_id) const
    {
        auto iter = accepted_opaques_.find(conn_id);
        NET_ASSERT(iter != accepted_opaques_.end(), "accepted conn:%lu not found", conn_id);
        return iter->second;
    }
    uint64_t OpaqueOfConnected(uint64_t conn_id) const
    {
        auto iter = connected_opaques_.find(conn_id);
        NET_ASSERT(iter != connected_opaques_.end(), "connected conn:%lu not found", conn_id);
        return iter->second;
    }
    uint64_t OpaqueOfRecv(uint64_t conn_id) const
    {
        auto iter = recv_opaques_.find(conn_id);
        NET_ASSERT(iter != recv_opaques_.end(), "recv conn:%lu not found", conn_id);
        return iter->second;
    }

    std::size_t PayloadCountOf(uint64_t conn_id) const
    {
        auto iter = recv_payloads_.find(conn_id);
        return iter == recv_payloads_.end() ? 0 : iter->second.size();
    }

    const std::vector<std::string>& PayloadsOf(uint64_t conn_id) const
    {
        static const std::vector<std::string> kEmpty;
        auto iter = recv_payloads_.find(conn_id);
        return iter == recv_payloads_.end() ? kEmpty : iter->second;
    }

    bool HasExactPayload(uint64_t conn_id, const std::string& expect) const
    {
        for (const std::string& payload : PayloadsOf(conn_id))
        {
            if (payload == expect)
            {
                return true;
            }
        }
        return false;
    }

    /*
     * UDP/KCP 的 OnReceived 拿到的是 Send 打包后的整帧(含 4 字节长度头).
     * 精确匹配请用 HasExactPayload(conn, MakeFramed(payload));
     * 本接口仅作宽松兜底(业务子串是否出现).
     */
    bool HasPayloadContaining(uint64_t conn_id, const std::string& expect) const
    {
        for (const std::string& payload : PayloadsOf(conn_id))
        {
            if (payload.find(expect) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<uint64_t> ConnectedConnIds() const
    {
        std::vector<uint64_t> result;
        result.reserve(connected_opaques_.size());
        for (const auto& item : connected_opaques_)
        {
            result.push_back(item.first);
        }
        return result;
    }

    std::vector<uint64_t> AcceptedConnIds() const
    {
        std::vector<uint64_t> result;
        result.reserve(accepted_opaques_.size());
        for (const auto& item : accepted_opaques_)
        {
            result.push_back(item.first);
        }
        return result;
    }

    void DumpState(const char* tag) const
    {
        fprintf(stderr, "[NetworkRegression] state(%s) binded:%u accepting:%u accepted:%u "
                "connected:%u connect_failed:%u errored:%u closed:%u received:%u "
                "last_err:%d last_sys_err:%d seq_len:%zu\n",
                tag, Count(ProbeEvent::Binded), Count(ProbeEvent::Accepting),
                Count(ProbeEvent::Accepted), Count(ProbeEvent::Connected),
                Count(ProbeEvent::ConnectFailed), Count(ProbeEvent::Errored),
                Count(ProbeEvent::Closed), Count(ProbeEvent::Received),
                static_cast<int32_t>(last_error_), last_sys_err_, event_seq_.size());
        for (std::size_t idx = 0; idx < event_seq_.size(); ++idx)
        {
            fprintf(stderr, "  seq[%zu] = %s\n", idx, ProbeEventName(event_seq_[idx]));
        }
        fflush(stderr);
    }

private:
    void Record(ProbeEvent event)
    {
        ++counters_[static_cast<std::size_t>(event)];
        event_seq_.push_back(event);
    }

    void BindCallbacks()
    {
        network_.SetOnBinded([this](ToolBox::NetworkType, uint64_t opaque, uint64_t conn_id,
                                    const std::string&, uint16_t)
        {
            Record(ProbeEvent::Binded);
            binded_opaques_[conn_id] = opaque;
        }).SetOnAccepting([this](ToolBox::NetworkType, uint64_t, int32_t fd)
        {
            Record(ProbeEvent::Accepting);
            last_accepting_fd_ = fd;
            accepting_fds_.push_back(fd);
            if (stop_in_accepting_)
            {
                // 验证 Accepting 回调内 StopWait:库侧不得再 Join,fd 必须由库关闭
                network_.StopWait();
            }
        }).SetOnAccepted([this](ToolBox::NetworkType, uint64_t opaque, uint64_t conn_id)
        {
            Record(ProbeEvent::Accepted);
            accepted_opaques_[conn_id] = opaque;
            if (stop_in_accepted_)
            {
                network_.StopWait();
            }
        }).SetOnConnected([this](ToolBox::NetworkType, uint64_t opaque, uint64_t conn_id)
        {
            Record(ProbeEvent::Connected);
            connected_opaques_[conn_id] = opaque;
            if (stop_in_connected_)
            {
                network_.StopWait();
            }
        }).SetOnConnectFailed([this](ToolBox::NetworkType, uint64_t,
                                     ToolBox::ENetErrCode err, int32_t sys_err)
        {
            Record(ProbeEvent::ConnectFailed);
            last_error_ = err;
            last_sys_err_ = sys_err;
        }).SetOnErrored([this](ToolBox::NetworkType, uint64_t, uint64_t,
                               ToolBox::ENetErrCode err, int32_t sys_err)
        {
            Record(ProbeEvent::Errored);
            last_error_ = err;
            last_sys_err_ = sys_err;
        }).SetOnClose([this](ToolBox::NetworkType, uint64_t, uint64_t conn_id,
                             ToolBox::ENetErrCode, int32_t)
        {
            Record(ProbeEvent::Closed);
            closed_order_.push_back(conn_id);
        }).SetOnReceived([this](ToolBox::NetworkType, uint64_t opaque, uint64_t conn_id,
                                const char* data, size_t size)
        {
            Record(ProbeEvent::Received);
            // size=0 时 data 可能为空指针,按空串记录
            if (0 == size || nullptr == data)
            {
                recv_payloads_[conn_id].emplace_back();
            }
            else
            {
                recv_payloads_[conn_id].emplace_back(data, data + size);
            }
            recv_opaques_[conn_id] = opaque;

            if (reenter_update_in_received_)
            {
                // 回调内重入 Update:只应标记延后,不得递归消费打乱事件顺序
                network_.Update();
                ++update_reenter_count_;
            }
            if (stop_in_received_)
            {
                network_.StopWait();
                return;
            }
            // 只有服务端连接回包,防止与客户端互相回显造成死循环
            if (server_echo_ && accepted_opaques_.count(conn_id) > 0)
            {
                const auto err = network_.Send(conn_id, data, static_cast<uint32_t>(size));
                NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == err,
                           "server echo failed conn:%lu err:%d", conn_id, static_cast<int32_t>(err));
            }
        });
    }

    ToolBox::Network& network_;

    bool server_echo_ = false;
    bool stop_in_accepting_ = false;
    bool stop_in_accepted_ = false;
    bool stop_in_connected_ = false;
    bool stop_in_received_ = false;
    bool reenter_update_in_received_ = false;

    std::vector<ProbeEvent> event_seq_;
    std::array<uint32_t, kProbeEventCount> counters_{};
    uint32_t update_reenter_count_ = 0;
    int32_t last_accepting_fd_ = -1;
    ToolBox::ENetErrCode last_error_ = ToolBox::ENetErrCode::NET_SUCCESS;
    int32_t last_sys_err_ = 0;

    std::vector<int32_t> accepting_fds_;
    std::vector<uint64_t> closed_order_;
    std::unordered_map<uint64_t, uint64_t> binded_opaques_;
    std::unordered_map<uint64_t, uint64_t> accepted_opaques_;
    std::unordered_map<uint64_t, uint64_t> connected_opaques_;
    std::unordered_map<uint64_t, std::vector<std::string>> recv_payloads_;
    std::unordered_map<uint64_t, uint64_t> recv_opaques_;
};

/*
 * 保证用例任何退出路径都停掉网络线程.StopWait 本身幂等,可与显式调用共存.
 * 必须声明在 NetworkProbe 之后:回调持有探针 this,须先 StopWait 再析构探针.
 */
class NetworkStopGuard
{
public:
    explicit NetworkStopGuard(ToolBox::Network& network)
        : network_(network)
    {
    }
    ~NetworkStopGuard()
    {
        network_.StopWait();
    }
    NetworkStopGuard(const NetworkStopGuard&) = delete;
    NetworkStopGuard& operator=(const NetworkStopGuard&) = delete;

private:
    ToolBox::Network& network_;
};

void RequireWait(NetworkProbe& probe, const std::function<bool()>& pred, const char* what,
                 int32_t timeout_ms = kDefaultTimeoutMs)
{
    if (probe.WaitUntil(pred, timeout_ms))
    {
        return;
    }
    fprintf(stderr, "[NetworkRegression] TIMEOUT waiting: %s\n", what);
    probe.DumpState(what);
    abort();
}

/*
 * 双 Network 实例(服务端/客户端各一)场景的等待:必须同时驱动两侧 Update.
 */
void RequireWaitBoth(NetworkProbe& server_probe, NetworkProbe& client_probe,
                     const std::function<bool()>& pred, const char* what,
                     int32_t timeout_ms = kDefaultTimeoutMs)
{
    const std::time_t begin = ToolBox::GetMillSecondTimeStamp();
    while (!pred())
    {
        server_probe.Pump();
        client_probe.Pump();
        if (pred())
        {
            return;
        }
        if (ToolBox::GetMillSecondTimeStamp() - begin > static_cast<std::time_t>(timeout_ms))
        {
            fprintf(stderr, "[NetworkRegression] TIMEOUT waiting: %s\n", what);
            server_probe.DumpState("server");
            client_probe.DumpState("client");
            abort();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
    }
}

/*
 * 三 Network 实例(如 1 服务端 + 2 客户端)场景的等待:必须同时驱动三侧 Update,
 * 否则未被 Pump 的实例回调无法推进,断言易误判/超时.
 */
void RequireWaitAll(NetworkProbe& a, NetworkProbe& b, NetworkProbe& c,
                    const std::function<bool()>& pred, const char* what,
                    int32_t timeout_ms = kDefaultTimeoutMs)
{
    const std::time_t begin = ToolBox::GetMillSecondTimeStamp();
    while (!pred())
    {
        a.Pump();
        b.Pump();
        c.Pump();
        if (pred())
        {
            return;
        }
        if (ToolBox::GetMillSecondTimeStamp() - begin > static_cast<std::time_t>(timeout_ms))
        {
            fprintf(stderr, "[NetworkRegression] TIMEOUT waiting: %s\n", what);
            a.DumpState("probe_a");
            b.DumpState("probe_b");
            c.DumpState("probe_c");
            abort();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
    }
}

void RequireSend(ToolBox::Network& network, uint64_t conn_id, const std::string& payload)
{
    // 空串勿用可能为空的 data(),避免 Send 内 memcpy(..., nullptr, 0) 触发 UB
    const char* data = payload.empty() ? "" : payload.data();
    const auto err = network.Send(conn_id, data, static_cast<uint32_t>(payload.size()));
    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == err,
               "send failed conn:%lu size:%zu err:%d",
               conn_id, payload.size(), static_cast<int32_t>(err));
}

/*
 * NetworkChannel::Send 统一加 4 字节长度头;TCP 收端会剥头,UDP/KCP 原样上送.
 * 对 UDP/KCP 做精确匹配时须用本函数拼出整帧期望值.
 */
std::string MakeFramed(const std::string& payload)
{
    const uint32_t size = static_cast<uint32_t>(payload.size());
    std::string framed;
    framed.resize(sizeof(uint32_t) + payload.size());
    memcpy(&framed[0], &size, sizeof(uint32_t));
    if (!payload.empty())
    {
        memcpy(&framed[sizeof(uint32_t)], payload.data(), payload.size());
    }
    return framed;
}

/*
 * 建立监听并等到 Binded 回调,返回监听连接 ID.
 * 必须等到 Binded 再 Connect,否则监听可能尚未完成 bind.
 */
uint64_t AcceptAndWaitBinded(ToolBox::Network& network, NetworkProbe& probe,
                             ToolBox::NetworkType type, uint64_t opaque, uint16_t port)
{
    network.Accept(type, opaque, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Binded) >= 1;
    }, "binded");
    return probe.FirstBinded();
}

/*
 * TCP 三方就绪:监听已 bind、服务端已 accepted、客户端已 connected.
 */
bool TcpPairReady(const NetworkProbe& probe)
{
    return probe.Count(ProbeEvent::Binded) >= 1
           && probe.Count(ProbeEvent::Accepted) >= 1
           && probe.Count(ProbeEvent::Connected) >= 1;
}

/*
 * 建立一对 TCP 连接,输出服务端/客户端 conn_id.
 */
void SetupTcpPair(ToolBox::Network& network, NetworkProbe& probe,
                  uint64_t server_opaque, uint64_t client_opaque,
                  uint64_t& listen_conn, uint64_t& server_conn, uint64_t& client_conn,
                  std::size_t net_thread_num = 1)
{
    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(net_thread_num), "Start failed thread_num:%zu", net_thread_num);
    listen_conn = AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, server_opaque, port);
    network.Connect(ToolBox::NT_TCP, client_opaque, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "tcp pair ready");
    server_conn = probe.FirstAccepted();
    client_conn = probe.FirstConnected();
    NET_ASSERT(server_conn != client_conn,
               "server/client conn must differ server:%lu client:%lu", server_conn, client_conn);
}

std::string MakeBinaryPayload()
{
    std::string payload;
    payload.reserve(64);
    for (int32_t i = 0; i < 64; ++i)
    {
        payload.push_back(static_cast<char>(i * 4));  // 覆盖 0x00 与高位字节
    }
    return payload;
}

}  // namespace

FIXTURE_BEGIN(NetworkRegression)

// ==================== 生命周期与参数校验 ====================

CASE(lifecycle_start_stop_idempotent)
{
    fprintf(stderr, "[NetworkRegression] lifecycle_start_stop_idempotent\n");
    ToolBox::Network network;
    NetworkStopGuard guard(network);

    NET_ASSERT(!network.Start(0), "Start(0) must be rejected");
    NET_ASSERT(network.Start(1), "first Start must succeed");
    NET_ASSERT(!network.Start(1), "duplicate Start must be rejected");
    NET_ASSERT(!network.Start(2), "duplicate Start with other thread num must be rejected");
    network.StopWait();

    // Stop 后必须能重新 Start,验证回滚与状态清理是幂等的
    NET_ASSERT(network.Start(2), "Start after StopWait must succeed");
    network.StopWait();
    // 重复 Stop 与停服后 Update 都必须安全
    network.StopWait();
    network.Update();
    network.Update();
}

CASE(invalid_conn_operations)
{
    fprintf(stderr, "[NetworkRegression] invalid_conn_operations\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    const std::string payload = "ping";

    // 未 Start 时的非法操作只返回错误码,不得崩溃
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == network.Close(0),
               "Close(0) before Start");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID
               == network.Send(0, payload.data(), 4), "Send(0) before Start");
    network.Update();

    NET_ASSERT(network.Start(1), "Start failed");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == network.Close(123456),
               "Close invalid conn after Start");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID
               == network.Send(123456, payload.data(), 4), "Send invalid conn after Start");
    probe.PumpIdle(5);
    NET_ASSERT(0 == probe.Count(ProbeEvent::Received), "unexpected received");
}

// ==================== TCP 握手、顺序与 opaque ====================

CASE(tcp_handshake_event_order_and_opaque)
{
    fprintf(stderr, "[NetworkRegression] tcp_handshake_event_order_and_opaque\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    const uint64_t server_opaque = 1001;
    const uint64_t client_opaque = 2002;
    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, server_opaque, client_opaque,
                 listen_conn, server_conn, client_conn);

    // 事件顺序: Binded 必须早于 Accepting,Accepting 必须早于 Accepted
    NET_ASSERT(probe.HappensBefore(ProbeEvent::Binded, ProbeEvent::Accepting),
               "Binded must happen before Accepting");
    NET_ASSERT(probe.HappensBefore(ProbeEvent::Accepting, ProbeEvent::Accepted),
               "Accepting must happen before Accepted");

    // 单客户端场景下各事件只应触发一次
    NET_ASSERT(1 == probe.Count(ProbeEvent::Binded),
               "Binded count:%u", probe.Count(ProbeEvent::Binded));
    NET_ASSERT(1 == probe.Count(ProbeEvent::Accepting),
               "Accepting count:%u", probe.Count(ProbeEvent::Accepting));
    NET_ASSERT(1 == probe.Count(ProbeEvent::Accepted),
               "Accepted count:%u", probe.Count(ProbeEvent::Accepted));
    NET_ASSERT(1 == probe.Count(ProbeEvent::Connected),
               "Connected count:%u", probe.Count(ProbeEvent::Connected));
    NET_ASSERT(0 == probe.Count(ProbeEvent::ConnectFailed), "unexpected ConnectFailed");
    NET_ASSERT(0 == probe.Count(ProbeEvent::Closed), "unexpected Closed");

    // Accepting 的 fd 必须有效,且交给库后业务不再持有
    NET_ASSERT(probe.last_accepting_fd() >= 0,
               "accepting fd invalid:%d", probe.last_accepting_fd());

    // opaque 必须原样透传: accepted 继承监听 opaque,connected 使用 Connect 的 opaque
    NET_ASSERT(server_opaque == probe.OpaqueOfBinded(listen_conn), "binded opaque mismatch");
    NET_ASSERT(server_opaque == probe.OpaqueOfAccepted(server_conn), "accepted opaque mismatch");
    NET_ASSERT(client_opaque == probe.OpaqueOfConnected(client_conn), "connected opaque mismatch");
}

CASE(tcp_echo_roundtrip_and_recv_opaque)
{
    fprintf(stderr, "[NetworkRegression] tcp_echo_roundtrip_and_recv_opaque\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    const uint64_t server_opaque = 1101;
    const uint64_t client_opaque = 2202;
    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, server_opaque, client_opaque,
                 listen_conn, server_conn, client_conn);

    const std::string payload = "hello-tcp-echo";
    RequireSend(network, client_conn, payload);

    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, payload)
               && probe.HasExactPayload(client_conn, payload);
    }, "tcp echo roundtrip");

    // OnReceived 的 opaque 必须是该连接自身的 opaque
    NET_ASSERT(server_opaque == probe.OpaqueOfRecv(server_conn), "server recv opaque mismatch");
    NET_ASSERT(client_opaque == probe.OpaqueOfRecv(client_conn), "client recv opaque mismatch");

    // echo 只由服务端发起,双方各自只应收到一个包
    NET_ASSERT(1 == probe.PayloadCountOf(server_conn),
               "server payload count:%zu", probe.PayloadCountOf(server_conn));
    NET_ASSERT(1 == probe.PayloadCountOf(client_conn),
               "client payload count:%zu", probe.PayloadCountOf(client_conn));
}

CASE(tcp_binary_and_large_payload)
{
    fprintf(stderr, "[NetworkRegression] tcp_binary_and_large_payload\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1201, 2301, listen_conn, server_conn, client_conn);

    // 含 0x00 的二进制内容必须按长度完整送达,不能被当作字符串截断
    const std::string binary = MakeBinaryPayload();
    RequireSend(network, client_conn, binary);
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, binary)
               && probe.HasExactPayload(client_conn, binary);
    }, "binary payload roundtrip");

    // 大包(远小于默认 256KB 缓冲上限)验证分片重组
    const std::string large(32 * 1024, 'L');
    RequireSend(network, client_conn, large);
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, large)
               && probe.HasExactPayload(client_conn, large);
    }, "large payload roundtrip");

    NET_ASSERT(0 == probe.Count(ProbeEvent::Closed),
               "connection must stay open, closed:%u", probe.Count(ProbeEvent::Closed));
}

CASE(tcp_multi_packet_order)
{
    fprintf(stderr, "[NetworkRegression] tcp_multi_packet_order\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1301, 2401, listen_conn, server_conn, client_conn);

    constexpr std::size_t kPacketNum = 32;
    std::vector<std::string> sent;
    sent.reserve(kPacketNum);
    for (std::size_t i = 0; i < kPacketNum; ++i)
    {
        std::string payload = "packet-" + std::to_string(i);
        RequireSend(network, client_conn, payload);
        sent.push_back(std::move(payload));
    }

    RequireWait(probe, [&]()
    {
        return probe.PayloadCountOf(server_conn) >= kPacketNum;
    }, "multi packet arrival");

    // 连续多包必须按发送顺序完整交付,不得粘包错位或丢包
    const std::vector<std::string>& received = probe.PayloadsOf(server_conn);
    NET_ASSERT(kPacketNum == received.size(),
               "received count:%zu expect:%zu", received.size(), kPacketNum);
    for (std::size_t i = 0; i < kPacketNum; ++i)
    {
        NET_ASSERT(received[i] == sent[i],
                   "packet order mismatch at:%zu got:%s expect:%s",
                   i, received[i].c_str(), sent[i].c_str());
    }
}

CASE(tcp_bidirectional_send)
{
    fprintf(stderr, "[NetworkRegression] tcp_bidirectional_send\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1401, 2501, listen_conn, server_conn, client_conn);

    // 不开 echo,双方各自主动发一包,验证收发方向互不干扰
    const std::string to_server = "client-to-server";
    const std::string to_client = "server-to-client";
    RequireSend(network, client_conn, to_server);
    RequireSend(network, server_conn, to_client);

    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, to_server)
               && probe.HasExactPayload(client_conn, to_client);
    }, "bidirectional delivery");

    NET_ASSERT(!probe.HasExactPayload(server_conn, to_client), "server got its own payload");
    NET_ASSERT(!probe.HasExactPayload(client_conn, to_server), "client got its own payload");
}

// ==================== TCP 关闭语义 ====================

CASE(tcp_close_client_conn)
{
    fprintf(stderr, "[NetworkRegression] tcp_close_client_conn\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1501, 2601, listen_conn, server_conn, client_conn);

    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == network.Close(client_conn),
               "Close client conn failed");
    RequireWait(probe, [&]()
    {
        return probe.IsClosed(client_conn);
    }, "client closed callback");

    // 关闭后 conn_id 已失效,再次 Send/Close 都应返回无效连接
    const std::string payload = "after-close";
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID
               == network.Send(client_conn, payload.data(),
                               static_cast<uint32_t>(payload.size())),
               "Send after Close must fail");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == network.Close(client_conn),
               "double Close must fail");

    // 对端应感知断开
    RequireWait(probe, [&]()
    {
        return probe.IsClosed(server_conn);
    }, "peer closed callback");
}

CASE(tcp_close_accepted_conn_by_server)
{
    fprintf(stderr, "[NetworkRegression] tcp_close_accepted_conn_by_server\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1601, 2701, listen_conn, server_conn, client_conn);

    // 服务端主动关闭 accepted 连接,双方都应收到 Closed,监听器不受影响
    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == network.Close(server_conn),
               "Close accepted conn failed");
    RequireWait(probe, [&]()
    {
        return probe.IsClosed(server_conn) && probe.IsClosed(client_conn);
    }, "both side closed");
    NET_ASSERT(!probe.IsClosed(listen_conn), "listener must stay open");
}

CASE(tcp_close_listener_keeps_established)
{
    fprintf(stderr, "[NetworkRegression] tcp_close_listener_keeps_established\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1701, 2801, listen_conn, server_conn, client_conn);

    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == network.Close(listen_conn),
               "Close listener failed");
    RequireWait(probe, [&]()
    {
        return probe.IsClosed(listen_conn);
    }, "listener closed callback");

    // 关闭监听器不应影响已建立连接的收发
    const std::string payload = "after-listener-close";
    RequireSend(network, client_conn, payload);
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, payload)
               && probe.HasExactPayload(client_conn, payload);
    }, "established conn still usable");
    NET_ASSERT(!probe.IsClosed(server_conn), "accepted conn must stay open");
    NET_ASSERT(!probe.IsClosed(client_conn), "client conn must stay open");
}

// ==================== 回调内 StopWait 与 Update 重入 ====================

CASE(tcp_stop_in_accepting_callback)
{
    fprintf(stderr, "[NetworkRegression] tcp_stop_in_accepting_callback\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableStopInAccepting(true);

    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 11, port);
    network.Connect(ToolBox::NT_TCP, 22, kLoopbackIp, port);

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Accepting) >= 1;
    }, "accepting callback");

    // 回调内已 StopWait:不得继续 Join,故 Accepted 不应触发
    NET_ASSERT(0 == probe.Count(ProbeEvent::Accepted),
               "Accepted must not fire after stop-in-accepting, accepted:%u",
               probe.Count(ProbeEvent::Accepted));
    NET_ASSERT(probe.last_accepting_fd() >= 0, "accepting fd invalid");

    // 停服后继续 Update 不应再产生任何回调
    const uint32_t snapshot = probe.TotalCallbackCount();
    probe.PumpIdle();
    NET_ASSERT(snapshot == probe.TotalCallbackCount(),
               "callback fired after StopWait, before:%u after:%u",
               snapshot, probe.TotalCallbackCount());

    // 打断后必须能重新 Start 并正常完成握手
    probe.Reset();
    probe.EnableStopInAccepting(false);
    NET_ASSERT(network.Start(1), "restart after stop-in-accepting failed");
    const uint16_t port2 = NextTestPort();
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 33, port2);
    network.Connect(ToolBox::NT_TCP, 44, kLoopbackIp, port2);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "restart tcp pair ready");
}

CASE(tcp_stop_in_received_callback)
{
    fprintf(stderr, "[NetworkRegression] tcp_stop_in_received_callback\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableStopInReceived(true);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1801, 2901, listen_conn, server_conn, client_conn);

    const std::string payload = "stop-on-recv";
    RequireSend(network, client_conn, payload);
    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Received) >= 1;
    }, "received callback");

    // 回调内 StopWait 后,剩余事件只回收不派发
    const uint32_t snapshot = probe.TotalCallbackCount();
    probe.PumpIdle();
    NET_ASSERT(snapshot == probe.TotalCallbackCount(),
               "callback fired after StopWait, before:%u after:%u",
               snapshot, probe.TotalCallbackCount());

    probe.Reset();
    probe.EnableStopInReceived(false);
    NET_ASSERT(network.Start(1), "restart after stop-in-received failed");
    const uint16_t port2 = NextTestPort();
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 55, port2);
    network.Connect(ToolBox::NT_TCP, 66, kLoopbackIp, port2);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "restart tcp pair ready");
}

CASE(tcp_update_reenter_in_callback)
{
    fprintf(stderr, "[NetworkRegression] tcp_update_reenter_in_callback\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableReenterUpdateInReceived(true);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 1901, 3001, listen_conn, server_conn, client_conn);

    constexpr std::size_t kPacketNum = 8;
    std::vector<std::string> sent;
    sent.reserve(kPacketNum);
    for (std::size_t i = 0; i < kPacketNum; ++i)
    {
        std::string payload = "reenter-" + std::to_string(i);
        RequireSend(network, client_conn, payload);
        sent.push_back(std::move(payload));
    }

    RequireWait(probe, [&]()
    {
        return probe.PayloadCountOf(server_conn) >= kPacketNum;
    }, "reenter packets arrival");

    NET_ASSERT(probe.update_reenter_count() >= kPacketNum,
               "reenter count:%u expect >= %zu", probe.update_reenter_count(), kPacketNum);

    // 回调内重入 Update 不得打乱事件顺序或重复消费
    const std::vector<std::string>& received = probe.PayloadsOf(server_conn);
    NET_ASSERT(kPacketNum == received.size(),
               "received count:%zu expect:%zu", received.size(), kPacketNum);
    for (std::size_t i = 0; i < kPacketNum; ++i)
    {
        NET_ASSERT(received[i] == sent[i],
                   "reenter order mismatch at:%zu got:%s expect:%s",
                   i, received[i].c_str(), sent[i].c_str());
    }
}

// ==================== Start 前缓存与 resume ====================

CASE(tcp_cached_events_before_start)
{
    fprintf(stderr, "[NetworkRegression] tcp_cached_events_before_start\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    const uint16_t port = NextTestPort();
    // Start 之前缓存 1 个 Accept 与 2 个 Connect,Start 后应按序补投
    network.Accept(ToolBox::NT_TCP, 701, kLoopbackIp, port);
    network.Connect(ToolBox::NT_TCP, 702, kLoopbackIp, port);
    network.Connect(ToolBox::NT_TCP, 703, kLoopbackIp, port);
    NET_ASSERT(network.Start(1), "Start with cached events failed");

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Binded) >= 1
               && probe.Count(ProbeEvent::Accepted) >= 2
               && probe.Count(ProbeEvent::Connected) >= 2;
    }, "cached events flush");

    // 缓存的 Accept 先于 Connect 入队,故监听必然先建立
    NET_ASSERT(probe.HappensBefore(ProbeEvent::Binded, ProbeEvent::Accepted),
               "Binded must happen before Accepted for cached events");

    // 连接集合此时已稳定,先快照避免在轮询谓词里反复构造
    const std::vector<uint64_t> clients = probe.ConnectedConnIds();
    NET_ASSERT(2 == clients.size(), "cached client count:%zu expect 2", clients.size());
    for (uint64_t client_conn : clients)
    {
        RequireSend(network, client_conn, "cached-" + std::to_string(client_conn));
    }
    RequireWait(probe, [&]()
    {
        for (uint64_t client_conn : clients)
        {
            if (!probe.HasExactPayload(client_conn, "cached-" + std::to_string(client_conn)))
            {
                return false;
            }
        }
        return true;
    }, "cached conns echo");
}

CASE(tcp_cache_resume_after_failed_start)
{
    fprintf(stderr, "[NetworkRegression] tcp_cache_resume_after_failed_start\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    const uint16_t port = NextTestPort();
    network.Accept(ToolBox::NT_TCP, 801, kLoopbackIp, port);
    network.Connect(ToolBox::NT_TCP, 802, kLoopbackIp, port);

    // Start 失败不得吞掉已缓存事件,修正参数后重新 Start 必须仍然生效
    NET_ASSERT(!network.Start(0), "Start(0) must fail");
    // 早退路径不应误报 Errored;缓存应静默保留供下次 Start 补投
    NET_ASSERT(0 == probe.Count(ProbeEvent::Errored),
               "Start(0) must keep cache without Errored, got:%u",
               probe.Count(ProbeEvent::Errored));
    NET_ASSERT(network.Start(1), "Start after failed Start must succeed");

    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "cached events resume after failed start");
    // 成功 resume 不应出现“线程号非法”类误报
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_NET_THREAD_INDEX != probe.last_error(),
               "resume must not report NET_INVALID_NET_THREAD_INDEX");
}

CASE(tcp_connect_failed_on_closed_port)
{
    fprintf(stderr, "[NetworkRegression] tcp_connect_failed_on_closed_port\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    const uint16_t unused_port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    // 未监听端口:loopback 通常立即 RST,留足超时以防个别环境 SYN 重试
    network.Connect(ToolBox::NT_TCP, 901, kLoopbackIp, unused_port);

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::ConnectFailed) >= 1
               || probe.Count(ProbeEvent::Errored) >= 1;
    }, "connect failed callback", kConnectFailTimeoutMs);

    NET_ASSERT(0 == probe.Count(ProbeEvent::Connected),
               "must not connect, connected:%u", probe.Count(ProbeEvent::Connected));
    NET_ASSERT(0 == probe.Count(ProbeEvent::Received), "must not receive");
}

CASE(tcp_connect_failed_no_storm)
{
    fprintf(stderr, "[NetworkRegression] tcp_connect_failed_no_storm\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    // 覆盖 UpdateError:CONNECTING 失败后必须 Close 摘除 epoll,否则 ERR 每轮唤醒刷屏
    const uint16_t unused_port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    network.Connect(ToolBox::NT_TCP, 911, kLoopbackIp, unused_port);

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::ConnectFailed) >= 1;
    }, "connect failed once", kConnectFailTimeoutMs);

    const uint32_t failed_at = probe.Count(ProbeEvent::ConnectFailed);
    const uint32_t errored_at = probe.Count(ProbeEvent::Errored);
    NET_ASSERT(failed_at >= 1, "expect ConnectFailed");
    // A1:CONNECTING 失败只 ConnectFailed,Close 静默回收,不应再有 Closed
    probe.PumpIdle(120);
    NET_ASSERT(0 == probe.Count(ProbeEvent::Closed),
               "connect fail must not Closed, closed:%u", probe.Count(ProbeEvent::Closed));

    // 空转:计数不得持续暴涨(允许迟到 1 次抖动)
    probe.PumpIdle(120);
    NET_ASSERT(probe.Count(ProbeEvent::ConnectFailed) <= failed_at + 1,
               "ConnectFailed storm: before:%u after:%u",
               failed_at, probe.Count(ProbeEvent::ConnectFailed));
    NET_ASSERT(probe.Count(ProbeEvent::Errored) <= errored_at + 2,
               "Errored storm: before:%u after:%u",
               errored_at, probe.Count(ProbeEvent::Errored));
    NET_ASSERT(0 == probe.Count(ProbeEvent::Closed), "Closed must stay 0");
    NET_ASSERT(0 == probe.Count(ProbeEvent::Connected), "must stay disconnected");
}

CASE(tcp_multi_client_isolation)
{
    fprintf(stderr, "[NetworkRegression] tcp_multi_client_isolation\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    constexpr std::size_t kClientNum = 3;
    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 1000, port);
    for (std::size_t i = 0; i < kClientNum; ++i)
    {
        network.Connect(ToolBox::NT_TCP, 2000 + i, kLoopbackIp, port);
    }

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Accepted) >= kClientNum
               && probe.Count(ProbeEvent::Connected) >= kClientNum;
    }, "multi client ready");

    const std::vector<uint64_t> clients = probe.ConnectedConnIds();
    NET_ASSERT(kClientNum == clients.size(),
               "client conn count:%zu expect:%zu", clients.size(), kClientNum);
    NET_ASSERT(kClientNum == probe.AcceptedConnIds().size(),
               "accepted conn count:%zu expect:%zu",
               probe.AcceptedConnIds().size(), kClientNum);

    // 每次 Accepting 上报的 fd 必须互不相同,否则说明 fd 被重复交割或提前回收
    const std::vector<int32_t>& fds = probe.accepting_fds();
    NET_ASSERT(kClientNum == fds.size(), "accepting fd count:%zu expect:%zu",
               fds.size(), kClientNum);
    std::unordered_set<int32_t> unique_fds(fds.begin(), fds.end());
    NET_ASSERT(kClientNum == unique_fds.size(),
               "accepting fd must be unique, unique:%zu expect:%zu",
               unique_fds.size(), kClientNum);

    // 每个客户端发唯一内容,回包必须一一对应,不得串连接
    for (uint64_t client_conn : clients)
    {
        RequireSend(network, client_conn, "client-" + std::to_string(client_conn));
    }
    RequireWait(probe, [&]()
    {
        for (uint64_t client_conn : clients)
        {
            if (!probe.HasExactPayload(client_conn, "client-" + std::to_string(client_conn)))
            {
                return false;
            }
        }
        return true;
    }, "multi client echo");

    for (uint64_t client_conn : clients)
    {
        NET_ASSERT(1 == probe.PayloadCountOf(client_conn),
                   "client:%lu payload count:%zu expect 1",
                   client_conn, probe.PayloadCountOf(client_conn));
        for (uint64_t other : clients)
        {
            if (other == client_conn)
            {
                continue;
            }
            NET_ASSERT(!probe.HasExactPayload(client_conn, "client-" + std::to_string(other)),
                       "client:%lu got payload of:%lu", client_conn, other);
        }
    }

    // 逐个关闭,互不影响
    for (uint64_t client_conn : clients)
    {
        NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == network.Close(client_conn),
                   "close client:%lu failed", client_conn);
    }
    RequireWait(probe, [&]()
    {
        for (uint64_t client_conn : clients)
        {
            if (!probe.IsClosed(client_conn))
            {
                return false;
            }
        }
        return true;
    }, "multi client closed");
}

// ==================== 扩展覆盖:多线程/停服/边界/Nagle ====================

CASE(tcp_multi_thread_pair)
{
    fprintf(stderr, "[NetworkRegression] tcp_multi_thread_pair\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    // Accept 固定线程 0,Connect 随机到 0/1,验证多 worker 路由与收发闭环
    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 3101, 3201, listen_conn, server_conn, client_conn, 2);

    const std::string payload = "multi-thread-echo";
    RequireSend(network, client_conn, payload);
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, payload)
               && probe.HasExactPayload(client_conn, payload);
    }, "multi thread echo");
}

CASE(tcp_stop_in_accepted_callback)
{
    fprintf(stderr, "[NetworkRegression] tcp_stop_in_accepted_callback\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableStopInAccepted(true);

    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 41, port);
    network.Connect(ToolBox::NT_TCP, 42, kLoopbackIp, port);

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Accepted) >= 1;
    }, "accepted callback");

    const uint32_t snapshot = probe.TotalCallbackCount();
    probe.PumpIdle();
    NET_ASSERT(snapshot == probe.TotalCallbackCount(),
               "callback fired after stop-in-accepted, before:%u after:%u",
               snapshot, probe.TotalCallbackCount());

    probe.Reset();
    probe.EnableStopInAccepted(false);
    NET_ASSERT(network.Start(1), "restart after stop-in-accepted failed");
    const uint16_t port2 = NextTestPort();
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 43, port2);
    network.Connect(ToolBox::NT_TCP, 44, kLoopbackIp, port2);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "restart after stop-in-accepted");
}

CASE(tcp_stop_in_connected_callback)
{
    fprintf(stderr, "[NetworkRegression] tcp_stop_in_connected_callback\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableStopInConnected(true);

    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 51, port);
    network.Connect(ToolBox::NT_TCP, 52, kLoopbackIp, port);

    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Connected) >= 1;
    }, "connected callback");

    const uint32_t snapshot = probe.TotalCallbackCount();
    probe.PumpIdle();
    NET_ASSERT(snapshot == probe.TotalCallbackCount(),
               "callback fired after stop-in-connected, before:%u after:%u",
               snapshot, probe.TotalCallbackCount());

    probe.Reset();
    probe.EnableStopInConnected(false);
    NET_ASSERT(network.Start(1), "restart after stop-in-connected failed");
    const uint16_t port2 = NextTestPort();
    AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 53, port2);
    network.Connect(ToolBox::NT_TCP, 54, kLoopbackIp, port2);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "restart after stop-in-connected");
}

CASE(tcp_bind_port_conflict)
{
    fprintf(stderr, "[NetworkRegression] tcp_bind_port_conflict\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    const uint16_t port = NextTestPort();
    NET_ASSERT(network.Start(1), "Start failed");
    const uint64_t listen_conn =
        AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, 61, port);

    // 同端口二次 Accept:应上报 NET_LISTEN_FAILED,首次监听仍可用
    network.Accept(ToolBox::NT_TCP, 62, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Errored) >= 1;
    }, "listen conflict errored");
    NET_ASSERT(ToolBox::ENetErrCode::NET_LISTEN_FAILED == probe.last_error(),
               "expect NET_LISTEN_FAILED, got:%d sys:%d",
               static_cast<int32_t>(probe.last_error()), probe.last_sys_err());
    NET_ASSERT(1 == probe.Count(ProbeEvent::Binded),
               "second Accept must not Binded, binded:%u", probe.Count(ProbeEvent::Binded));

    network.Connect(ToolBox::NT_TCP, 63, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "first listener still usable");
    NET_ASSERT(!probe.IsClosed(listen_conn), "first listener must stay open");
}

CASE(tcp_ops_after_stopwait)
{
    fprintf(stderr, "[NetworkRegression] tcp_ops_after_stopwait\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 71, 72, listen_conn, server_conn, client_conn);

    network.StopWait();
    // StopWait 清空 conn_type_,旧 conn 的 Send/Close 必须失败
    const std::string payload = "after-stop";
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID
               == network.Send(client_conn, payload.data(),
                               static_cast<uint32_t>(payload.size())),
               "Send after StopWait must fail");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == network.Close(client_conn),
               "Close after StopWait must fail");
    network.Update();

    // Accept 在未 Start 时走缓存路径(resume),不得崩溃;再次 Start 后应可完成握手
    probe.Reset();
    const uint16_t port2 = NextTestPort();
    network.Accept(ToolBox::NT_TCP, 73, kLoopbackIp, port2);
    network.Connect(ToolBox::NT_TCP, 74, kLoopbackIp, port2);
    NET_ASSERT(network.Start(1), "Start after StopWait ops failed");
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "resume after stopwait ops");
}

CASE(tcp_empty_and_single_byte)
{
    fprintf(stderr, "[NetworkRegression] tcp_empty_and_single_byte\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 81, 82, listen_conn, server_conn, client_conn);

    // 现状:size=0 合法,打包 4 字节长度头后对端 OnReceived(size=0)
    // 不要传 nullptr,避免 memcpy(dest,nullptr,0) 的 UB
    const std::string empty;
    RequireSend(network, client_conn, empty);
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(server_conn, empty);
    }, "empty payload received");
    NET_ASSERT(1 == probe.PayloadCountOf(server_conn),
               "empty payload count:%zu", probe.PayloadCountOf(server_conn));
    NET_ASSERT(0 == probe.PayloadsOf(server_conn).front().size(),
               "empty payload size must be 0");

    const std::string one_byte(1, '\x7f');
    RequireSend(network, client_conn, one_byte);
    RequireWait(probe, [&]()
    {
        return probe.PayloadCountOf(server_conn) >= 2
               && probe.HasExactPayload(server_conn, one_byte);
    }, "single byte received");
    NET_ASSERT(probe.PayloadsOf(server_conn)[0].empty(), "first payload must stay empty");
    NET_ASSERT(probe.PayloadsOf(server_conn)[1] == one_byte, "second payload mismatch");
}

CASE(tcp_rapid_reconnect)
{
    fprintf(stderr, "[NetworkRegression] tcp_rapid_reconnect\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);
    probe.EnableServerEcho(true);

    const uint16_t port = NextTestPort();
    const uint64_t server_opaque = 91;
    NET_ASSERT(network.Start(1), "Start failed");
    const uint64_t listen_conn =
        AcceptAndWaitBinded(network, probe, ToolBox::NT_TCP, server_opaque, port);

    network.Connect(ToolBox::NT_TCP, 92, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return TcpPairReady(probe);
    }, "first pair ready");
    const uint64_t first_client = probe.FirstConnected();
    const uint64_t first_server = probe.FirstAccepted();

    RequireSend(network, first_client, "round-1");
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(first_client, "round-1");
    }, "first round echo");

    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == network.Close(first_client),
               "close first client failed");
    RequireWait(probe, [&]()
    {
        return probe.IsClosed(first_client) && probe.IsClosed(first_server);
    }, "first pair closed");
    NET_ASSERT(!probe.IsClosed(listen_conn), "listener must survive reconnect");

    // 同端口再建连:监听器复用,conn_id/opaque 不得串到已关闭连接
    network.Connect(ToolBox::NT_TCP, 93, kLoopbackIp, port);
    RequireWait(probe, [&]()
    {
        return probe.Count(ProbeEvent::Connected) >= 2
               && probe.Count(ProbeEvent::Accepted) >= 2;
    }, "second pair ready");

    uint64_t second_client = 0;
    for (uint64_t conn : probe.ConnectedConnIds())
    {
        if (conn != first_client)
        {
            second_client = conn;
            break;
        }
    }
    NET_ASSERT(0 != second_client, "second client conn not found");
    NET_ASSERT(93 == probe.OpaqueOfConnected(second_client),
               "second client opaque mismatch");

    RequireSend(network, second_client, "round-2");
    RequireWait(probe, [&]()
    {
        return probe.HasExactPayload(second_client, "round-2");
    }, "second round echo");
    NET_ASSERT(!probe.HasExactPayload(first_client, "round-2"),
               "closed client must not receive new payload");
}

CASE(tcp_nagle_batch_send)
{
    fprintf(stderr, "[NetworkRegression] tcp_nagle_batch_send\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 101, 102, listen_conn, server_conn, client_conn);

    // 累计 3 包才刷出;timeout 给大值,避免干扰包数路径
    network.SetSimulateNagle(3, 5000);
    // SetSimulateNagle 异步投递到 worker,空转尽量让参数生效后再发
    probe.PumpIdle(80);

    RequireSend(network, client_conn, "nagle-0");
    RequireSend(network, client_conn, "nagle-1");
    // hold 窗口:若 Nagle 未生效会立刻到达,据此抓假阳性
    const std::time_t hold_begin = ToolBox::GetMillSecondTimeStamp();
    while (ToolBox::GetMillSecondTimeStamp() - hold_begin < 50)
    {
        probe.Pump();
        NET_ASSERT(0 == probe.PayloadCountOf(server_conn),
                   "nagle must hold before 3rd packet, got:%zu",
                   probe.PayloadCountOf(server_conn));
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
    }

    RequireSend(network, client_conn, "nagle-2");
    RequireWait(probe, [&]()
    {
        return probe.PayloadCountOf(server_conn) >= 3;
    }, "nagle flush by packet count");

    const std::vector<std::string>& received = probe.PayloadsOf(server_conn);
    NET_ASSERT(3 == received.size(), "nagle received:%zu expect 3", received.size());
    NET_ASSERT(received[0] == "nagle-0", "nagle order 0 mismatch");
    NET_ASSERT(received[1] == "nagle-1", "nagle order 1 mismatch");
    NET_ASSERT(received[2] == "nagle-2", "nagle order 2 mismatch");
}

CASE(tcp_nagle_timeout_flush)
{
    fprintf(stderr, "[NetworkRegression] tcp_nagle_timeout_flush\n");
    ToolBox::Network network;
    NetworkProbe probe(network);
    NetworkStopGuard guard(network);

    uint64_t listen_conn = 0;
    uint64_t server_conn = 0;
    uint64_t client_conn = 0;
    SetupTcpPair(network, probe, 111, 112, listen_conn, server_conn, client_conn);

    // 包数阈值故意设很大(永远凑不满),只能靠 timeout(60ms)刷出,专测超时路径
    network.SetSimulateNagle(1000, 60);
    probe.PumpIdle(80);

    const std::time_t send_begin = ToolBox::GetMillSecondTimeStamp();
    RequireSend(network, client_conn, "timeout-0");
    RequireSend(network, client_conn, "timeout-1");
    // 前 30ms 不应到达:若配置未生效会立刻发送,据此抓假阴性
    while (ToolBox::GetMillSecondTimeStamp() - send_begin < 30)
    {
        probe.Pump();
        NET_ASSERT(0 == probe.PayloadCountOf(server_conn),
                   "nagle timeout must hold initially, elapsed:%lld got:%zu",
                   static_cast<long long>(ToolBox::GetMillSecondTimeStamp() - send_begin),
                   probe.PayloadCountOf(server_conn));
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollSleepMs));
    }
    RequireWait(probe, [&]()
    {
        return probe.PayloadCountOf(server_conn) >= 2;
    }, "nagle flush by timeout");

    const std::vector<std::string>& received = probe.PayloadsOf(server_conn);
    NET_ASSERT(2 == received.size(), "nagle timeout received:%zu expect 2", received.size());
    NET_ASSERT(received[0] == "timeout-0", "nagle timeout order 0 mismatch");
    NET_ASSERT(received[1] == "timeout-1", "nagle timeout order 1 mismatch");
}

// ==================== UDP(仅 Linux 支持) ====================
//
// UDP 的 conn_id 由对端地址(ip<<32|port)派生,而不是连接池下标.因此在同一个
// Network 实例内做 loopback 自连接时,ACCEPTOR 的"本地地址 ID"与指向它的
// CONNECTOR 的"远端地址 ID"完全相同,会在 address_to_connect_ 中互相覆盖,
// 导致 Close/Send 路由到错误的 socket.以下用例统一拆成服务端/客户端两个
// Network 实例,各自持有独立映射表,才能真实验证 UDP 语义.
//
// GetNetThreadIndex 取 conn_id 的 16~23 位,UDP 地址 ID 这些位恒为 0;
// Connect 已强制 UDP/KCP 落 0 号线程.常规用例 Start(1);udp_start2_routing 覆盖 Start(2).

#if defined(__linux__)

CASE(udp_echo_roundtrip)
{
    fprintf(stderr, "[NetworkRegression] udp_echo_roundtrip\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    const uint64_t server_opaque = 501;
    const uint64_t client_opaque = 502;
    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");

    server.Accept(ToolBox::NT_UDP, server_opaque, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded");
    const uint64_t acceptor_conn = server_probe.FirstBinded();
    NET_ASSERT(server_opaque == server_probe.OpaqueOfBinded(acceptor_conn),
               "udp binded opaque mismatch");

    client.Connect(ToolBox::NT_UDP, client_opaque, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "udp connector connected");
    const uint64_t client_conn = client_probe.FirstConnected();
    NET_ASSERT(client_opaque == client_probe.OpaqueOfConnected(client_conn),
               "udp connected opaque mismatch");

    // 客户端首包让服务端 ACCEPTOR 创建 REMOTE 并回调 Accepted
    const std::string payload = "hello-udp";
    RequireSend(client, client_conn, payload);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1
               && server_probe.Count(ProbeEvent::Received) >= 1;
    }, "udp server accepted and received");

    const uint64_t remote_conn = server_probe.FirstAccepted();
    NET_ASSERT(acceptor_conn != remote_conn, "acceptor and remote conn must differ");
    // REMOTE 的 opaque 必须继承 ACCEPTOR
    NET_ASSERT(server_opaque == server_probe.OpaqueOfAccepted(remote_conn),
               "udp remote opaque must inherit acceptor");
    NET_ASSERT(server_probe.HasExactPayload(remote_conn, MakeFramed(payload)),
               "udp server did not get exact framed payload");
    NET_ASSERT(server_opaque == server_probe.OpaqueOfRecv(remote_conn),
               "udp server recv opaque mismatch");

    // 服务端按 REMOTE 地址回包
    const std::string reply = "reply-udp";
    RequireSend(server, remote_conn, reply);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.HasExactPayload(client_conn, MakeFramed(reply));
    }, "udp client got reply");
    NET_ASSERT(client_opaque == client_probe.OpaqueOfRecv(client_conn),
               "udp client recv opaque mismatch");

    // CONNECTOR 侧的对端地址已在映射表中,不应再触发 Accepted
    NET_ASSERT(0 == client_probe.Count(ProbeEvent::Accepted),
               "client must not accept, accepted:%u", client_probe.Count(ProbeEvent::Accepted));
}

CASE(udp_multi_client_isolation)
{
    fprintf(stderr, "[NetworkRegression] udp_multi_client_isolation\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    // 服务端起两个 ACCEPTOR,客户端各连一个:客户端侧 conn_id 取自远端地址,
    // 若两个 CONNECTOR 连同一端口会得到相同 conn_id 而互相覆盖,故必须用不同端口.
    const uint16_t port_a = NextTestPort();
    const uint16_t port_b = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");
    server.Accept(ToolBox::NT_UDP, 601, kLoopbackIp, port_a);
    server.Accept(ToolBox::NT_UDP, 602, kLoopbackIp, port_b);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 2;
    }, "two udp acceptors binded");

    client.Connect(ToolBox::NT_UDP, 611, kLoopbackIp, port_a);
    client.Connect(ToolBox::NT_UDP, 612, kLoopbackIp, port_b);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 2;
    }, "two udp connectors connected");

    const std::vector<uint64_t> clients = client_probe.ConnectedConnIds();
    NET_ASSERT(2 == clients.size(), "udp client count:%zu expect 2", clients.size());

    for (uint64_t client_conn : clients)
    {
        RequireSend(client, client_conn, "udp-" + std::to_string(client_conn));
    }
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 2
               && server_probe.Count(ProbeEvent::Received) >= 2;
    }, "two udp remotes accepted");

    // 两条链路的 conn_id 与 opaque 必须彼此独立
    std::unordered_set<uint64_t> client_opaques;
    for (uint64_t client_conn : clients)
    {
        client_opaques.insert(client_probe.OpaqueOfConnected(client_conn));
    }
    NET_ASSERT(2 == client_opaques.size(),
               "udp client opaque must be distinct, got:%zu", client_opaques.size());

    const std::vector<uint64_t> remotes = server_probe.AcceptedConnIds();
    NET_ASSERT(2 == remotes.size(), "udp remote count:%zu expect 2", remotes.size());

    // 服务端分别回包,客户端各自只应收到属于自己的内容
    for (uint64_t remote_conn : remotes)
    {
        RequireSend(server, remote_conn, "srv-" + std::to_string(remote_conn));
    }
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        for (uint64_t client_conn : clients)
        {
            if (0 == client_probe.PayloadCountOf(client_conn))
            {
                return false;
            }
        }
        return true;
    }, "two udp clients got reply");

    // 每个客户端只应收到一份回包,且两份内容互不相同,证明没有串链路
    std::unordered_set<std::string> replies;
    for (uint64_t client_conn : clients)
    {
        NET_ASSERT(1 == client_probe.PayloadCountOf(client_conn),
                   "udp client:%lu payload count:%zu expect 1",
                   client_conn, client_probe.PayloadCountOf(client_conn));
        replies.insert(client_probe.PayloadsOf(client_conn).front());
    }
    NET_ASSERT(2 == replies.size(),
               "udp replies must be distinct, got:%zu", replies.size());
}

CASE(udp_acceptor_cascade_close)
{
    fprintf(stderr, "[NetworkRegression] udp_acceptor_cascade_close\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");
    server.Accept(ToolBox::NT_UDP, 701, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded");
    const uint64_t acceptor_conn = server_probe.FirstBinded();

    client.Connect(ToolBox::NT_UDP, 702, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "udp connector connected");

    // 先让服务端产生一个共享 fd 的 REMOTE
    RequireSend(client, client_probe.FirstConnected(), "cascade");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1;
    }, "udp remote accepted");
    const uint64_t remote_conn = server_probe.FirstAccepted();

    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == server.Close(acceptor_conn),
               "close udp acceptor failed");

    // ACCEPTOR 关闭必须级联关闭共享其 fd 的 REMOTE,两者都要回调 Closed
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.IsClosed(acceptor_conn) && server_probe.IsClosed(remote_conn);
    }, "udp cascade close");

    // 级联语义: REMOTE 的 Closed 先入队,ACCEPTOR 最后
    const std::size_t remote_idx = server_probe.ClosedIndexOf(remote_conn);
    const std::size_t acceptor_idx = server_probe.ClosedIndexOf(acceptor_conn);
    NET_ASSERT(remote_idx < acceptor_idx,
               "remote closed must precede acceptor, remote_idx:%zu acceptor_idx:%zu",
               remote_idx, acceptor_idx);

    // 已关闭的 conn_id 必须失效
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == server.Close(acceptor_conn),
               "double close acceptor must fail");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == server.Close(remote_conn),
               "close cascaded remote must fail");
}

CASE(udp_close_remote_keeps_acceptor)
{
    fprintf(stderr, "[NetworkRegression] udp_close_remote_keeps_acceptor\n");
    ToolBox::Network server;
    ToolBox::Network client_a;
    ToolBox::Network client_b;
    NetworkProbe server_probe(server);
    NetworkProbe client_a_probe(client_a);
    NetworkProbe client_b_probe(client_b);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_a_guard(client_a);
    NetworkStopGuard client_b_guard(client_b);

    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client_a.Start(1), "client_a Start failed");
    NET_ASSERT(client_b.Start(1), "client_b Start failed");

    server.Accept(ToolBox::NT_UDP, 801, kLoopbackIp, port);
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded");
    const uint64_t acceptor_conn = server_probe.FirstBinded();

    client_a.Connect(ToolBox::NT_UDP, 802, kLoopbackIp, port);
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return client_a_probe.Count(ProbeEvent::Connected) >= 1;
    }, "client_a connected");
    RequireSend(client_a, client_a_probe.FirstConnected(), "from-a");
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1;
    }, "remote_a accepted");
    const uint64_t remote_a = server_probe.FirstAccepted();

    // 只关 REMOTE,ACCEPTOR 必须存活
    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == server.Close(remote_a),
               "close remote failed");
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return server_probe.IsClosed(remote_a);
    }, "remote_a closed");
    NET_ASSERT(!server_probe.IsClosed(acceptor_conn), "acceptor must stay open");

    // 另一客户端仍可触发新的 REMOTE
    client_b.Connect(ToolBox::NT_UDP, 803, kLoopbackIp, port);
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return client_b_probe.Count(ProbeEvent::Connected) >= 1;
    }, "client_b connected");
    RequireSend(client_b, client_b_probe.FirstConnected(), "from-b");
    RequireWaitAll(server_probe, client_a_probe, client_b_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 2
               && server_probe.Count(ProbeEvent::Received) >= 2;
    }, "remote_b accepted after remote_a closed");
    NET_ASSERT(!server_probe.IsClosed(acceptor_conn),
               "acceptor must stay open after second remote");
}

CASE(udp_connector_close)
{
    fprintf(stderr, "[NetworkRegression] udp_connector_close\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");
    server.Accept(ToolBox::NT_UDP, 901, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded");
    const uint64_t acceptor_conn = server_probe.FirstBinded();

    client.Connect(ToolBox::NT_UDP, 902, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "udp connector connected");
    const uint64_t client_conn = client_probe.FirstConnected();
    RequireSend(client, client_conn, "before-close");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1;
    }, "udp remote accepted");

    // 客户端关 CONNECTOR 不得拖垮服务端 ACCEPTOR
    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == client.Close(client_conn),
               "close connector failed");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.IsClosed(client_conn);
    }, "connector closed");
    NET_ASSERT(!server_probe.IsClosed(acceptor_conn),
               "acceptor must not close with connector");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID == client.Close(client_conn),
               "double close connector must fail");
    NET_ASSERT(ToolBox::ENetErrCode::NET_INVALID_CONNID
               == client.Send(client_conn, "x", 1),
               "Send on closed connector must fail");
}

CASE(kcp_echo_roundtrip)
{
    fprintf(stderr, "[NetworkRegression] kcp_echo_roundtrip\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    // KCP 与 UDP 同地址 ID 约束,必须双实例;KCP 可靠收发应拿到完整业务载荷
    const uint64_t server_opaque = 1001;
    const uint64_t client_opaque = 1002;
    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");

    server.Accept(ToolBox::NT_KCP, server_opaque, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "kcp acceptor binded");
    NET_ASSERT(server_opaque == server_probe.OpaqueOfBinded(server_probe.FirstBinded()),
               "kcp binded opaque mismatch");

    client.Connect(ToolBox::NT_KCP, client_opaque, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "kcp connector connected");
    const uint64_t client_conn = client_probe.FirstConnected();
    NET_ASSERT(client_opaque == client_probe.OpaqueOfConnected(client_conn),
               "kcp connected opaque mismatch");

    const std::string payload = "hello-kcp";
    RequireSend(client, client_conn, payload);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1
               && server_probe.Count(ProbeEvent::Received) >= 1;
    }, "kcp server accepted and received");

    const uint64_t remote_conn = server_probe.FirstAccepted();
    NET_ASSERT(server_opaque == server_probe.OpaqueOfAccepted(remote_conn),
               "kcp remote opaque must inherit acceptor");
    // KCP 保留消息边界,但收端拿到的是 Send 整帧(含 4 字节长度头)
    NET_ASSERT(server_probe.HasExactPayload(remote_conn, MakeFramed(payload)),
               "kcp server did not get exact framed payload");

    const std::string reply = "reply-kcp";
    RequireSend(server, remote_conn, reply);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.HasExactPayload(client_conn, MakeFramed(reply));
    }, "kcp client got reply");
}

CASE(udp_empty_payload)
{
    fprintf(stderr, "[NetworkRegression] udp_empty_payload\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(1), "server Start failed");
    NET_ASSERT(client.Start(1), "client Start failed");
    server.Accept(ToolBox::NT_UDP, 1101, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded");
    const uint64_t acceptor_conn = server_probe.FirstBinded();

    client.Connect(ToolBox::NT_UDP, 1102, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "udp connector connected");
    const uint64_t client_conn = client_probe.FirstConnected();

    // 先用真实包建立 REMOTE
    RequireSend(client, client_conn, "warmup");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1
               && server_probe.Count(ProbeEvent::Received) >= 1;
    }, "udp remote accepted");
    const uint64_t remote_conn = server_probe.FirstAccepted();
    const std::size_t recv_before = server_probe.PayloadCountOf(remote_conn);

    // 现状: NetworkChannel::Send(size=0) 仍打包 4 字节长度头下发,UDP 收端原样上送
    // MakeFramed(""),并非 socket 层 length==0 的 no-op.连接应保持健康.
    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == client.Send(client_conn, "", 0),
               "udp empty send should return success");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.PayloadCountOf(remote_conn) >= recv_before + 1
               && server_probe.HasExactPayload(remote_conn, MakeFramed(""));
    }, "udp empty framed payload delivered");
    NET_ASSERT(recv_before + 1 == server_probe.PayloadCountOf(remote_conn),
               "udp empty should deliver exactly one framed packet, before:%zu after:%zu",
               recv_before, server_probe.PayloadCountOf(remote_conn));
    NET_ASSERT(!server_probe.IsClosed(remote_conn), "udp empty send must not close remote");
    NET_ASSERT(!server_probe.IsClosed(acceptor_conn), "udp empty send must not close acceptor");

    // 空包后链路仍健康:真实包依旧能送达
    RequireSend(client, client_conn, "after-empty");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.HasExactPayload(remote_conn, MakeFramed("after-empty"));
    }, "udp real packet after empty");
}

CASE(udp_start2_routing)
{
    fprintf(stderr, "[NetworkRegression] udp_start2_routing\n");
    ToolBox::Network server;
    ToolBox::Network client;
    NetworkProbe server_probe(server);
    NetworkProbe client_probe(client);
    NetworkStopGuard server_guard(server);
    NetworkStopGuard client_guard(client);

    // 多网络线程下,UDP 的 Accept 固定落 0 号线程,Connect 亦被强制到 0 号线程,
    // 保证 conn_id(地址派生,线程位恒 0)与 socket 同线程,Send/Close 路由不错位.
    const uint16_t port = NextTestPort();
    NET_ASSERT(server.Start(2), "server Start(2) failed");
    NET_ASSERT(client.Start(2), "client Start(2) failed");

    server.Accept(ToolBox::NT_UDP, 1201, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Binded) >= 1;
    }, "udp acceptor binded on start2");

    client.Connect(ToolBox::NT_UDP, 1202, kLoopbackIp, port);
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.Count(ProbeEvent::Connected) >= 1;
    }, "udp connector connected on start2");
    const uint64_t client_conn = client_probe.FirstConnected();

    RequireSend(client, client_conn, "start2-req");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return server_probe.Count(ProbeEvent::Accepted) >= 1
               && server_probe.Count(ProbeEvent::Received) >= 1;
    }, "udp server received on start2");
    const uint64_t remote_conn = server_probe.FirstAccepted();
    NET_ASSERT(server_probe.HasExactPayload(remote_conn, MakeFramed("start2-req")),
               "udp server payload mismatch on start2");

    // 回包与关闭同样必须能正确路由回 0 号线程
    RequireSend(server, remote_conn, "start2-rsp");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.HasExactPayload(client_conn, MakeFramed("start2-rsp"));
    }, "udp client got reply on start2");

    NET_ASSERT(ToolBox::ENetErrCode::NET_SUCCESS == client.Close(client_conn),
               "close connector on start2 failed");
    RequireWaitBoth(server_probe, client_probe, [&]()
    {
        return client_probe.IsClosed(client_conn);
    }, "connector closed on start2");
}

#else

CASE(udp_cases_skipped_on_this_platform)
{
    fprintf(stderr, "[NetworkRegression] UDP/KCP cases are linux-only, skipped\n");
}

#endif  // __linux__

FIXTURE_END(NetworkRegression)
