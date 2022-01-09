# pragma once
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <array>
#include <string>
#include <memory>

namespace ToolBox{

// 实现五层时间轮算法

/*
*  | _ _ _ _ _ _ | _ _ _ _ _ _ | _ _ _ _ _ _ | _ _ _ _ _ _ | _ _ _ _ _ _ _ _ |
* BITS  TVN(6)        TVN(6)        TVN(6)        TVN(6)          TVR(8)
* SIZE   64            64            64            64              256
*/
using HTIMER = uint64_t;
constexpr uint32_t TVN_BITS = 6;                                        // 6位二进制
constexpr uint32_t TVR_BITS = 8;                                        // 8位二进制
constexpr uint32_t TVN_SIZE = 1 << TVN_BITS;                            // 64 外层时间轮的slot数目
constexpr uint32_t TVR_SIZE = 1 << TVR_BITS;                            // 256 一层时间轮的slot数目
constexpr uint32_t TVN_MASK = TVN_SIZE - 1;                             // 63-->00111111 外层时间轮slot的最大索引值 掩码
constexpr uint32_t TVR_MASK = TVR_SIZE - 1;                             // 255-->11111111 一层时间轮slot的最大索引值 掩码
#define OFFSET(N) (TVR_SIZE + (N) * TVN_SIZE)                           // 第N+1层时间轮在数组中的起始位置 N>=0
#define INDEX(V,N) ((V >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)      // 定时器节点在当前层级的数组位置 N>=0
constexpr uint32_t MAX_SLOT = 256 + 4 * 64;                             // 数组大小
constexpr uint32_t INVALID_HTIMER = 0;

/*
* 定义定时器任务接口
*/
class ITimer
{
public:
    /*
    * 虚析构
    */
    virtual ~ITimer(){}
    /*
    * @brief 触发函数
    * @param id 定时器ID
    * @param count 当前触发次数
    */
    virtual void OnTimer(uint32_t id, uint32_t count) = 0;
};

/*
* 定义参数接口
*/
class IArgs
{
public:
    virtual ~IArgs(){}
};

using TMethod = std::function<bool(std::weak_ptr<IArgs>, std::weak_ptr<void>)>;
#define DelegateCombination(T_, Func_, Instance_) (ToolBox::XDelegate::RegisterMethod(std::bind(&T_::Func_, Instance_, std::placeholders::_1, std::placeholders::_2)))

/*
* 定义委托
*/
class XDelegate
{
public:
    XDelegate()
    : stub_ptr_(nullptr)
    {}
    
    static XDelegate RegisterMethod(const TMethod& method)
    {
        XDelegate xd;
        xd.stub_ptr_ = method;
        return xd;
    }

    bool operator()(std::weak_ptr<IArgs> pargs, std::weak_ptr<void> arg) const
    {
        if(nullptr == stub_ptr_)
        {
            return false;
        }
        return stub_ptr_(pargs, arg);
    }
private:
    TMethod stub_ptr_;
};


/*
* 定义定时器接口
*/
class ITimerWheel
{
public:
    /*
    * 虚析构
    */
    virtual ~ITimerWheel(){}
    /*
    * @brief 增加定时器
    * @param timer 定时器回调接口
    * @param id 定时器ID
    * @param interval 定时间隔,毫秒为单位
    * @param count 触发次数, -1为永远触发
    * @return 成功返回 Timer的句柄,失败返回 INVALID_HTIMER
    */
    virtual HTIMER AddTimer(std::weak_ptr<ITimer> timer, int32_t id, int32_t interval, int32_t count, const std::string& filename = "", int32_t lineno = 0) = 0;
    /*
    * @brief 增加定时器
    * @param delegate 定时器委托
    * @param args 定时任务执行对象参数
    * @param arg 定时任务参数
    * @param interval 定时间隔,毫秒为单位
    * @param count 触发次数, -1为永远触发
    * @return 成功返回 Timer的句柄,失败返回 INVALID_HTIMER
    */
    virtual HTIMER AddTimer(const XDelegate& delegate, std::weak_ptr<IArgs> args, std::weak_ptr<void> arg, int32_t interval, int32_t count, const std::string& filename = "", int32_t lineno = 0) = 0;

    /*
    * @brief Timer过多长时间后会触发
    * @param timer 句柄
    */
    virtual int32_t GetTimeLeft(HTIMER timer) = 0;
    /*
    * @brief 关闭定时器
    */
    virtual void KillTimer(HTIMER timer) = 0;
    /*
    * @brief 主循环里面需要不停的调用Update
    */
    virtual void Update() = 0;
    /*
    * @brief 释放
    */
    virtual void Release() = 0;
};

/*
* @brief 定义触发类型.内部类型
*        根据外部使用接口的不同,内部赋予不同的触发类型
*/
enum class ETriggerType
{
    ETRIGGER_INVALID = 0,   // 定义无效类型,用于初始状态
    ETRIGGER_ONTIMER = 1,   // 定义 ITIMER 类型, 此种类型对象需要继承ITimer
    ETRIGGER_DELEGATE = 2,  // 定义委托类型
};


/*
* 定义定时器节点
*/
struct TimerNode
{
    TimerNode* prev = nullptr;      // 前置节点
    TimerNode* next = nullptr;      // 后置节点
    uint32_t identifier = 0;        // 透传定时器ID
    ETriggerType trigger_type = ETriggerType::ETRIGGER_INVALID; // 定义触发类型
    std::weak_ptr<ITimer> timer;    // 定时器接口
    XDelegate delegate;             // 委托事件
    std::weak_ptr<IArgs> delegate_args;  // 委托事件参数
    std::weak_ptr<void> args;           // 透传参数
    int64_t expire_time = 0;        // 超时时间
    HTIMER guid = 0;                // 唯一标识
    int32_t interval = 0;           // 间隔
    int32_t total_count = 0;        // 计数器
    int32_t curr_count = 0;         // 当前次数

    std::string file;               // 文件名
    int32_t line = 0;               // 行号

    /*
    * 析构
    */
    ~TimerNode()
    {
        Reset();
    }
    /*
    * 重置数据
    */
    void Reset()
    {
        trigger_type = ETriggerType::ETRIGGER_INVALID;
        prev = nullptr;
        next = nullptr;
        timer.reset();
        delegate_args.reset();
        args.reset();
        guid = 0;
        interval = 0;
        total_count = 0;
        curr_count = 0;
        file="";
        line = 0;
        expire_time = 0;
    }
    /*
    * @brief 初始化链表
    * @param  node 哨兵节点
    * @return void
    */
    inline void ListInit(TimerNode* node)
    {
        if(nullptr == node)
        {
            return;
        }
        node->prev = nullptr;
        node->next = nullptr;
    }
    /*
    * @brief 添加节点
    * @param cur_node 添加节点位置
    * @param new_node 待添加的节点
    * @return void
    */
   static void ListAdd(TimerNode* cur_node, TimerNode* new_node)
   {
       if(nullptr == cur_node || nullptr == new_node)
       {
           return;
       }
       new_node -> next = cur_node -> next;
       new_node -> prev = cur_node;
       cur_node -> next -> prev = new_node;
       cur_node -> next = new_node;
   }
   /*
   * @brief 删除节点
   * @param node 要被删除的节点
   * @param void
   */
   static void ListRemove(TimerNode* node)
   {
       if(nullptr == node)
       {
           return;
       }
       node -> next -> prev = node -> prev;
       node -> prev -> next = node -> next;
   }
};

/*
* 定时时间轮
*/
class TimerWheel : public ITimerWheel
{
public:
    /*
    * 构造
    */
    TimerWheel();
    /*
    * 析构
    */
    virtual ~TimerWheel();
    /*
    * @brief 添加定时器任务
    * @param timer 定时器接口
    * @param id identifier
    * @param interval 间隔时间
    * @param count 触发次数
    * @param file 文件名
    * @param line 文件号
    */
    HTIMER AddTimer(std::weak_ptr<ITimer> timer, int32_t id, int32_t interval, int32_t count, const std::string &file = "", int32_t line = 0) override;
    /*
    * @brief 添加定时器任务 
    * @param callback 定时器回调
    * @param delegate_args 委托参数
    * @param args 透传参数
    * @param interval 间隔时间
    * @param count 触发次数
    * @param file 文件名
    * @param line 文件号
    */
    HTIMER AddTimer(const TMethod& callback, std::weak_ptr<IArgs> delegate_args, std::weak_ptr<void> args, int32_t interval, int32_t count, const std::string file = "", int32_t line = 0);
    /*
    * @brief 添加定时器任务 
    * @param delegate 类函数接口委托
    * @param delegate_args 委托参数
    * @param args 透传参数
    * @param interval 间隔时间
    * @param count 触发次数
    * @param file 文件名
    * @param line 文件号
    */
    HTIMER AddTimer(const XDelegate& delegate, std::weak_ptr<IArgs> delegate_args, std::weak_ptr<void> args, int32_t interval, int32_t count, const std::string &file = "", int32_t line = 0) override;
    /*
    * @brief Timer过多长时间后会触发
    * @param timer 句柄
    * @return 返回剩余毫秒数
    */
    int32_t GetTimeLeft(HTIMER timer) override;
    /*
    * @brief 删除一个定时器节点
    * @return timer 定时器句柄
    */
    void KillTimer(HTIMER timer) override;
    /*
    * @brief 释放定时器
    * @return void
    */
    void Release() override;
    /*
    * @brief 执行一次更新
    * @return void
    */
    void Update() override;
private:
    /*
    * @brief 初始化时间轮
    * @return void
    */
    void Init();
    /*
    * @brief 释放时间轮
    * @return void
    */
    void UnInit();
    /*
    * @brief 获取一个新的ID
    * @return HTIMER
    */
    HTIMER GetNewTimerID()
    {
        return ++next_id_;
    }
    /*
    * @brief 获取空闲的节点
    */
    TimerNode* GetFreeNode();
    /*
    * @brief 添加空闲节点
    */
    void AddFreeNode(TimerNode* node);
    /*
    * @brief 触发节点
    */
    void OnNodeTrigger(TimerNode* node);
    /*
    * @brief 添加定时器
    */
    void AddTimer(TimerNode* node);
    /*
    * @brief 外层的时间节点向内层转移.
    */
    int32_t CascadeTime(int32_t off, int32_t index);
    /*
    * @brief 获取毫秒数
    */
    uint64_t GetMilliSecond();
private: 
    using TimersMap = std::unordered_map<HTIMER, TimerNode*>;
    using FreeNode = std::deque<TimerNode*>;
    using TimersArray = std::array<TimerNode, MAX_SLOT>;

    TimersMap all_timers_;              // 可以通过句柄查找所有定时器节点的容器
    TimersArray timer_nodes_;           // 维护5层时间轮的数组
    FreeNode free_nodes_;               // 已经删除
    HTIMER next_id_ = 0;                // 下一个可以使用的句柄
    int64_t cur_time_ = 0;              // 时间轮当前时间
    int64_t last_update_time_ = 0;      // 上次更新时间
};

TimerWheel::TimerWheel()
{
    Init();
}
TimerWheel:: ~TimerWheel()
{
    UnInit();
}

HTIMER TimerWheel::AddTimer(std::weak_ptr<ITimer> timer, int32_t id, int32_t interval, int32_t count, const std::string &file, int32_t line)
{
    std::shared_ptr<ITimer> sp_timer = timer.lock();
    if(nullptr == sp_timer)
    {
        return INVALID_HTIMER;
    }

    auto* node = GetFreeNode();
    if(nullptr == node)
    {
        return INVALID_HTIMER;
    }
    node->trigger_type = ETriggerType::ETRIGGER_ONTIMER;
    node->timer = timer;
    node->identifier = id;
    node->interval = interval;
    node->total_count = count;
    node->file = file;
    node->line = line;
    if(node->interval >= INT32_MAX)
    {
        node->interval = INT32_MAX;
    }
    node->expire_time = cur_time_ + node->interval;
    node->guid = GetNewTimerID();
    AddTimer(node);
    return node->guid;
}

HTIMER TimerWheel::AddTimer(const TMethod& callback, std::weak_ptr<IArgs> delegate_args, std::weak_ptr<void> args, int32_t interval, int32_t count, const std::string file, int32_t line)
{
    if(nullptr == callback)
    {
        return INVALID_HTIMER;
    }
    return AddTimer(XDelegate::RegisterMethod(callback), delegate_args, args, interval, count, file, line);
}

HTIMER TimerWheel::AddTimer(const XDelegate& delegate, std::weak_ptr<IArgs> delegate_args, std::weak_ptr<void> args, int32_t interval, int32_t count, const std::string &file, int32_t line)
{
    auto* node = GetFreeNode();
    if(nullptr == node)
    {
        return INVALID_HTIMER;
    }
    node->trigger_type = ETriggerType::ETRIGGER_DELEGATE;
    node->timer.reset();
    node->delegate = delegate;
    node->delegate_args = delegate_args;
    node->args = args;
    node->interval = interval;
    node->total_count = count;
    node->file = file;
    node->line = line;
    if(node->interval >= INT32_MAX)
    {
        node->interval = INT32_MAX;
    }
    node->expire_time = cur_time_ + node->interval;
    node->guid = GetNewTimerID();
    AddTimer(node);
    return node->guid;
}

int32_t TimerWheel::GetTimeLeft(HTIMER timer)
{
    auto iter = all_timers_.find(timer);
    if(iter == all_timers_.end())
    {
        return 0;
    }
    return static_cast<uint32_t>(iter->second->expire_time - cur_time_);
}

void TimerWheel::KillTimer(HTIMER timer)
{
    auto iter = all_timers_.find(timer);
    if(iter == all_timers_.end())
    {
        return;
    }
    auto* node = iter->second;
    if(nullptr == node)
    {
        return;
    }
    TimerNode::ListRemove(node);
    all_timers_.erase(iter);
    AddFreeNode(node);
}

void TimerWheel::Release()
{
    UnInit();
}

void TimerWheel::Update()
{
    // 获取现在的毫秒数
    int64_t now_time = GetMilliSecond();
    if(0 == last_update_time_)
    {
        // 第一次 Update 只更新 last_update_time_
        last_update_time_ = now_time;
        return;
    }
    // 时间流逝的毫秒数
    int64_t time_delta = now_time - last_update_time_;
    if(0 == time_delta)
    {
        return;
    }
    // 用"现在的毫秒数" 更新 "上次更新时间"
    last_update_time_ = now_time;   
    // 设置时间轮时钟的最新值
    int64_t new_time = cur_time_ + time_delta;
    while(cur_time_ < new_time)
    {
        int32_t idx = cur_time_ & TVR_MASK;
        // 当前时间低8位如果为0,则把外层时间轮上的时间节点往内层转移
        if(0 == idx
            && 0 == CascadeTime(OFFSET(0), INDEX(cur_time_, 0))
            && 0 == CascadeTime(OFFSET(1), INDEX(cur_time_, 1))
            && 0 == CascadeTime(OFFSET(2), INDEX(cur_time_, 2)))
            {
                CascadeTime(OFFSET(3), INDEX(cur_time_, 3));
            }
        auto* node = &timer_nodes_[idx];
        while(node -> next != node)
        {
            auto* next = node->next;
            ++next->curr_count;
            OnNodeTrigger(next);
            TimerNode::ListRemove(next);
            if(next -> total_count < 0 || next -> curr_count < next -> total_count)
            {
                next -> expire_time = cur_time_ + next -> interval;
                AddTimer(next);
            } else
            {
                all_timers_.erase(next->guid);
                AddFreeNode(next);
            }
        }
        cur_time_++;
    }
}

void TimerWheel::Init()
{
    for(auto &iter : timer_nodes_)
    {
        auto* head = &iter;
        head -> next = head;
        head -> prev = head;
    }
}

void TimerWheel::UnInit()
{
    for(auto& iter : timer_nodes_)
    {
        auto* node = &iter;
        while(node -> next != node)
        {
            auto* node_to_del = node -> next;
            TimerNode::ListRemove(node_to_del);
            AddFreeNode(node_to_del);
        }
    }
    for(auto &p : free_nodes_)
    {
        delete p;
        p = nullptr;
    }
    all_timers_.clear();
    free_nodes_.clear();
}

TimerNode* TimerWheel::GetFreeNode()
{
    if(free_nodes_.empty())
    {
        return new TimerNode();
    }
    auto* node = free_nodes_.front();
    free_nodes_.pop_front();
    return node;
}

void TimerWheel::AddFreeNode(TimerNode* node)
{
    if(nullptr == node)
    {
        return;
    }
    node -> Reset();
    free_nodes_.emplace_back(node);
}

void TimerWheel::OnNodeTrigger(TimerNode* node)
{
    if(nullptr == node)
    {
        return;
    }
    switch (node->trigger_type)
    {
    case ETriggerType::ETRIGGER_ONTIMER:
        {
            auto sp_timer = node->timer.lock();
            if(nullptr != sp_timer)
            {
                sp_timer->OnTimer(node->identifier, node->curr_count);
            } 
            break;
        }
    case ETriggerType::ETRIGGER_DELEGATE:
    {
        node->delegate(node -> delegate_args, node -> args);
        break;
    }
    default:
        break;
    }     
}

void TimerWheel::AddTimer(TimerNode* node)
{
    if(nullptr == node)
    {
        return;
    }
    int32_t slot_idx = 0;
    int64_t delay = node -> expire_time - cur_time_;
    if(delay < 0)
    {
        // 已经超时的定时器放入最内层时间轮的随机位置
        slot_idx = cur_time_ & TVR_MASK;
    } else if(delay < TVR_SIZE)
    {
        // 第一层的索引为低8位的值
        slot_idx = node -> expire_time & TVR_MASK;
    } else if(delay < (1 << (TVR_BITS + TVN_BITS)))
    {
        // 第二层的索引偏移量+9-14位值
        slot_idx = OFFSET(0) + INDEX(node->expire_time, 0);
    } else if(delay < (1 << (TVR_BITS + 2 * TVN_BITS)))
    {
        // 第三层的索引为偏移值+15-20位值
        slot_idx = OFFSET(1) + INDEX(node->expire_time, 1);
    } else if(delay < (1 << (TVR_BITS + 3 * TVN_BITS)))
    {
        // 第四层的索引为偏移量+21-26位值
        slot_idx = OFFSET(2) + INDEX(node->expire_time, 2);
    } else
    {
        // 最外层的索引为偏移量+27-32位值
        slot_idx = OFFSET(3) + INDEX(node->expire_time,3);
    }
    
    all_timers_[node->guid] = node;
    auto* head = &timer_nodes_[slot_idx];
    TimerNode::ListAdd(head, node);
}

int32_t TimerWheel::CascadeTime(int32_t off, int32_t index)
{
    int32_t slot_idx = off + index;
    auto* node = &timer_nodes_[slot_idx];
    while(node->next != node)
    {
        auto* next = node -> next;
        TimerNode::ListRemove(next);
        AddTimer(next);
    }
    return index;
}

uint64_t TimerWheel::GetMilliSecond()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    return ms.count();
}

/*
* 定义定时器管理器单件
*/
#define TimerMgr Singleton<ToolBox::TimerWheel>::Instance()

};  // ToolBox












// Kafka 中 TimingWheel 源码关于时间轮的讲解
/*
 * Hierarchical Timing Wheels
 *
 * A simple timing wheel is a circular list of buckets of timer tasks. Let u be the time unit.
 * A timing wheel with size n has n buckets and can hold timer tasks in n * u time interval.
 * Each bucket holds timer tasks that fall into the corresponding time range. At the beginning,
 * the first bucket holds tasks for [0, u), the second bucket holds tasks for [u, 2u), …,
 * the n-th bucket for [u * (n -1), u * n). Every interval of time unit u, the timer ticks and
 * moved to the next bucket then expire all timer tasks in it. So, the timer never insert a task
 * into the bucket for the current time since it is already expired. The timer immediately runs
 * the expired task. The emptied bucket is then available for the next round, so if the current
 * bucket is for the time t, it becomes the bucket for [t + u * n, t + (n + 1) * u) after a tick.
 * A timing wheel has O(1) cost for insert/delete (start-timer/stop-timer) whereas priority queue
 * based timers, such as java.util.concurrent.DelayQueue and java.util.Timer, have O(log n)
 * insert/delete cost.
 *
 * A major drawback of a simple timing wheel is that it assumes that a timer request is within
 * the time interval of n * u from the current time. If a timer request is out of this interval,
 * it is an overflow. A hierarchical timing wheel deals with such overflows. It is a hierarchically
 * organized timing wheels. The lowest level has the finest time resolution. As moving up the
 * hierarchy, time resolutions become coarser. If the resolution of a wheel at one level is u and
 * the size is n, the resolution of the next level should be n * u. At each level overflows are
 * delegated to the wheel in one level higher. When the wheel in the higher level ticks, it reinsert
 * timer tasks to the lower level. An overflow wheel can be created on-demand. When a bucket in an
 * overflow bucket expires, all tasks in it are reinserted into the timer recursively. The tasks
 * are then moved to the finer grain wheels or be executed. The insert (start-timer) cost is O(m)
 * where m is the number of wheels, which is usually very small compared to the number of requests
 * in the system, and the delete (stop-timer) cost is still O(1).
 *
 * Example
 * Let's say that u is 1 and n is 3. If the start time is c,
 * then the buckets at different levels are:
 *
 * level    buckets
 * 1        [c,c]   [c+1,c+1]  [c+2,c+2]
 * 2        [c,c+2] [c+3,c+5]  [c+6,c+8]
 * 3        [c,c+8] [c+9,c+17] [c+18,c+26]
 *
 * The bucket expiration is at the time of bucket beginning.
 * So at time = c+1, buckets [c,c], [c,c+2] and [c,c+8] are expired.
 * Level 1's clock moves to c+1, and [c+3,c+3] is created.
 * Level 2 and level3's clock stay at c since their clocks move in unit of 3 and 9, respectively.
 * So, no new buckets are created in level 2 and 3.
 *
 * Note that bucket [c,c+2] in level 2 won't receive any task since that range is already covered in level 1.
 * The same is true for the bucket [c,c+8] in level 3 since its range is covered in level 2.
 * This is a bit wasteful, but simplifies the implementation.
 *
 * 1        [c+1,c+1]  [c+2,c+2]  [c+3,c+3]
 * 2        [c,c+2]    [c+3,c+5]  [c+6,c+8]
 * 3        [c,c+8]    [c+9,c+17] [c+18,c+26]
 *
 * At time = c+2, [c+1,c+1] is newly expired.
 * Level 1 moves to c+2, and [c+4,c+4] is created,
 *
 * 1        [c+2,c+2]  [c+3,c+3]  [c+4,c+4]
 * 2        [c,c+2]    [c+3,c+5]  [c+6,c+8]
 * 3        [c,c+8]    [c+9,c+17] [c+18,c+18]
 *
 * At time = c+3, [c+2,c+2] is newly expired.
 * Level 2 moves to c+3, and [c+5,c+5] and [c+9,c+11] are created.
 * Level 3 stay at c.
 *
 * 1        [c+3,c+3]  [c+4,c+4]  [c+5,c+5]
 * 2        [c+3,c+5]  [c+6,c+8]  [c+9,c+11]
 * 3        [c,c+8]    [c+9,c+17] [c+8,c+11]
 *
 * The hierarchical timing wheels works especially well when operations are completed before they time out.
 * Even when everything times out, it still has advantageous when there are many items in the timer.
 * Its insert cost (including reinsert) and delete cost are O(m) and O(1), respectively while priority
 * queue based timers takes O(log N) for both insert and delete where N is the number of items in the queue.
 *
 * This class is not thread-safe. There should not be any add calls while advanceClock is executing.
 * It is caller's responsibility to enforce it. Simultaneous add calls are thread-safe.
 */

