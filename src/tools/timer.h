# pragma once
#include <stdint.h>
#include <functional>

// 实现五层时间轮算法

using HTIMER = uint64_t;
#define INVALID_HTIMER 0

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

using TMethod = std::function<bool(IArgs*, void*)>;
#define DelegateCombination(T_, Func_, Instance_) (XDelegate::RegisterMethod<T_, TMethod>(std::bind(&T_::Func_, Instance_, std::placeholders::_1, std::placeholders::_2)))

/*
* 定义定时器事件
*/
class XDelegate
{
public:
    XDelegate()
    : stub_ptr_(nullptr)
    {}
    
    template<class TMethod>
    XDelegate RegisterMethod(TMethod method)
    {
        XDelegate xd;
        // xd.object_ptr_ = object_ptr;
        xd.stub_ptr_ = method;
        return xd;
    }

private:
   
    template<class TMethod>
    bool MethodStub(IArgs* pargs, void* arg)
    {
        return stub_ptr_(pargs, arg);
    }
private:
    // using StubType = std::function<bool(void* object_ptr, IArgs*, void* arg)>;
    TMethod stub_ptr_;

};

/*
* 定义定时器接口
*/
class ITimerManager
{
public:
    /*
    * 虚析构
    */
    virtual ~ITimerManager(){}
    /*
    * @brief 增加定时器
    * @param timer 定时器回调接口
    * @param id 定时器ID
    * @param interval 定时间隔,毫秒为单位
    * @param count 触发次数, -1为永远触发
    * @return 成功返回 Timer的句柄,失败返回 INVALID_HTIMER
    */
    virtual HTIMER SetTimer(ITimer* timer, uint32_t id, uint32_t interval, uint32_t count, const char* filename, int32_t lineno) = 0;
    /*
    * @brief 增加定时器
    * @param delegate 定时器委托
    * @param args 定时任务执行对象参数
    * @param arg 定时任务参数
    * @param interval 定时间隔,毫秒为单位
    * @param count 触发次数, -1为永远触发
    * @return 成功返回 Timer的句柄,失败返回 INVALID_HTIMER
    */
    virtual HTIMER SetTimer(const XDelegate& delegate, IArgs *args, void *arg, uint32_t interval, uint32_t count,const char* filename, int32_t lineno) = 0;

    /*
    * @brief Timer过多长时间后会触发
    * @param timer 句柄
    */
    virtual uint32_t GetTimeLeft(HTIMER timer) = 0;
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

