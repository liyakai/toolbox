# pragma once
#include <stdint.h>
#include <functional>

// ʵ�����ʱ�����㷨

using HTIMER = uint64_t;
#define TVN_BITS 6                                                  // 6λ������
#define TVR_BITS 8                                                  // 8λ������
#define TVN_SIZE (1 << TVN_BITS)                                    // 64 ���ʱ���ֵ�slot��Ŀ
#define TVR_SIZE (1 << TVN_SIZE)                                    // 256 һ��ʱ���ֵ�slot��Ŀ
#define TVN_MASK (TVN_SIZE - 1)                                     // 63-->00111111 ���ʱ����slot���������ֵ ����
#define TVR_MASK (TVR_SIZE - 1)                                     // 255-->11111111 һ��ʱ����slot���������ֵ ����
#define OFFSET(N) (TVR_SIZE + (N) * TVN_SIZE)                       // ��N+1��ʱ�����������е���ʼλ�� N>=0
#define INDEX(V,N) ((V >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)  // ��ʱ���ڵ��ڵ�ǰ�㼶������λ�� N>=0
#define MAX_SLOT 256 + 4 * 64                                       // �����С
#define INVALID_HTIMER 0



/*
* ���嶨ʱ������ӿ�
*/
class ITimer
{
public:
    /*
    * ������
    */
    virtual ~ITimer(){}
    /*
    * @brief ��������
    * @param id ��ʱ��ID
    * @param count ��ǰ��������
    */
    virtual void OnTimer(uint32_t id, uint32_t count) = 0;
};

/*
* ��������ӿ�
*/
class IArgs
{
public:
    virtual ~IArgs(){}
};

using TMethod = std::function<bool(IArgs*, void*)>;
#define DelegateCombination(T_, Func_, Instance_) (XDelegate::RegisterMethod<T_, TMethod>(std::bind(&T_::Func_, Instance_, std::placeholders::_1, std::placeholders::_2)))

/*
* �����ຯ���󶨽ӿ�
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
        xd.stub_ptr_ = method;
        return xd;
    }

    bool operator()(IArgs* pargs, void* arg) const
    {
        return stub_ptr_(pargs, arg);
    }
private:
    TMethod stub_ptr_;
};

/*
* ���嶨ʱ���ڵ�
*/
struct TimerNode
{
    TimerNode* prev = nullptr;      // ǰ�ýڵ�
    TimerNode* next = nullptr;      // ���ýڵ�
    XDelegate delegate;             // ί���¼�
    IArgs* delegate_args = nullptr;  // ί���¼�����
    void* args = nullptr;           // ͸������
    int64_t expire_time = 0;        // ��ʱʱ��
    HTIMER guid = 0;                // Ψһ��ʶ
    int32_t interval = 0;           // ���
    int32_t total_count = 0;        // ������
    int32_t curr_count = 0;         // ��ǰ����

    std::string file;               // �ļ���
    int32_t line = 0;               // �к�

    /*
    * ����
    */
    ~TimerNode()
    {
        Reset();
    }
    /*
    * ��������
    */
    void Reset()
    {
        if(delegate_args)
        {
            delete delegate_args;
            delegate_args = nullptr;
        }
        prev = nullptr;
        next = nullptr;
        args = nullptr;
        guid = 0;
        interval = 0;
        total_count = 0;
        curr_count = 0;
        file="";
        line = 0;
        expire_time = 0;
    }
    /*
    * @brief ��ʼ������
    * @param  node �ڱ��ڵ�
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
    * @brief ��ӽڵ�
    * @param cur_node ��ӽڵ�λ��
    * @param new_node ����ӵĽڵ�
    * @return void
    */
   inline void ListAdd(TimerNode* cur_node, TimerNode* new_node)
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
   * @brief ɾ���ڵ�
   * @param node Ҫ��ɾ���Ľڵ�
   * @param void
   */
   inline void ListRemove(TimerNode* node)
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
* ��ʱʱ����
*/
class TimerWheel
{
public:
    /*
    * ����
    */
    TimerWheel()
    {
        Init();
    }
    /*
    * ����
    */
    ~TimerWheel()
    {
        UnInit();
    }
    /*
    * @brief ��Ӷ�ʱ�� 
    * @param delegate �ຯ���ӿ�ί��
    * @param delegate_args ί�в���
    * @param args ͸������
    * @param interval ���ʱ��
    * @param count ��������
    * @param file �ļ���
    * @param line �ļ���
    */
    HTIMER AddTimer(const XDelegate& delegate, IArgs* delegate_args, void* args, int32_t interval, int32_t count, const string file, int32_t line)
    {

    }

private:
    using TimersMap = std::unordered_map<HTIMER, TimerNode*>;
    using FreeNode = std::deque<TimerNode*>;
    using TimersArray = std::array<TimerNode, MAX_SLOT>;
    /*
    * @brief ��ʼ��ʱ����
    * @return void
    */
    void Init()
    {
        for(auto &iter : timer_nodes_)
        {
            auto* head = &iter;
            head -> next = head;
            head -> prev = head;
        }
    }
    /*
    * @brief �ͷ�ʱ����
    * @return void
    */
    void UnInit()
    {
        for(auto& iter : TimerWheel)
        {
            auto* node = &iter;
            while(node -> next != node)
            {
                auto* node_to_del = node -> next;
                ListRemove(node);
                AddFreeNode(node);
            }
        }
        for(auto &p : free_nodes)
        {
            delete p;
            p = nullptr;
        }
        all_timers_.clear();
        free_nodes_.clear();
    }
    /*
    * @brief ��ȡ���еĽڵ�
    */
    TimerNode* GetFreeNode()
    {
        if(free_nodes.empty())
        {
            return new TimerNode();
        }
        auto* node = free_nodes.front();
        free_nodes.pop_front();
        return node;
    }
    /*
    * @brief ��ӿ��нڵ�
    */
    void AddFreeNodew(TimerNode* node)
    {
        if(nullptr != node)
        {
            return;
        }
        node -> Reset();
        free_nodes.emplace_back(node);
    }
    /*
    * @brief ��Ӷ�ʱ��
    */
    void AddTimer(TimerNode* node)
    {
        if(nullptr != node)
        {
            return;
        }
        int32_t slot_idx = 0;
        int64_t delay = node -> expire_time - cur_time_;
        if(delay < TVR_SIZE)
        {
            // ��һ�������Ϊ��8λ��ֵ
            slot_idx = node -> expire_time & TVR_MASK;
        } else if(delay < (1 << TVR_BITS + TVR_BITS))
        {
            // �ڶ��������ƫ����+9-14λֵ
            slot_idx = OFFSET(0) + INDEX(node->expire_time, 0);
        } else if(delay < (1 << (TVR_BITS + 2 * TVN_BITS)))
        {
            // �����������Ϊƫ��ֵ+15-20λֵ
            slot_idx = OFFSET(1) + INDEX(node->expire_time, 1);
        } else if(delay < (1 << (TVR_BITS + 3 * TVN_BITS)))
        {
            // ���Ĳ������Ϊƫ����+21-26λֵ
            slot_idx = OFFSET(2) + INDEX(node->expire_time, 2);
        } else if(delay < 0)
        {
            // �Ѿ���ʱ�Ķ�ʱ���������ڲ�ʱ���ֵ����λ��
            slot_idx = cur_time_ & TVR_MASK;
        } else
        {
            // ����������Ϊƫ����+27-32λֵ
            slot_idx = OFFSET(3) + INDEX(node->expire_time,3);
        }
        auto* head = &timer_nodes_[slot_idx];
        if(all_timers_.emplace(node->guid, node).second)
        {
            ListAdd(head, node);
        }
    }
    /*
    * @brief 
    */
    int32_t CascadeTime(int32_t off, int32_t index)
    {
        int32_t slot_idx = off + index;
        auto* node = &timer_nodes_[slot_idx];
        while(node->next != node)
        {
            auto* next = node -> next;
            ListRemove(next);
            AddTimer(next);
        }
    }
private: 
    TimersMap all_timers_;              // ����ͨ������������ж�ʱ���ڵ������
    TimersArray timer_nodes_;           // ά��5��ʱ���ֵ�����
    FreeNode free_nodes_;               // �Ѿ�ɾ��
    HTIMER next_id_ = 0;                // ��һ������ʹ�õľ��
    int64_t cur_time_ = 0;              // ʱ���ֵ�ǰʱ��
}

/*
* ���嶨ʱ���ӿ�
*/
class ITimerManager
{
public:
    /*
    * ������
    */
    virtual ~ITimerManager(){}
    /*
    * @brief ���Ӷ�ʱ��
    * @param timer ��ʱ���ص��ӿ�
    * @param id ��ʱ��ID
    * @param interval ��ʱ���,����Ϊ��λ
    * @param count ��������, -1Ϊ��Զ����
    * @return �ɹ����� Timer�ľ��,ʧ�ܷ��� INVALID_HTIMER
    */
    virtual HTIMER SetTimer(ITimer* timer, uint32_t id, uint32_t interval, uint32_t count, const char* filename, int32_t lineno) = 0;
    /*
    * @brief ���Ӷ�ʱ��
    * @param delegate ��ʱ��ί��
    * @param args ��ʱ����ִ�ж������
    * @param arg ��ʱ�������
    * @param interval ��ʱ���,����Ϊ��λ
    * @param count ��������, -1Ϊ��Զ����
    * @return �ɹ����� Timer�ľ��,ʧ�ܷ��� INVALID_HTIMER
    */
    virtual HTIMER SetTimer(const XDelegate& delegate, IArgs *args, void *arg, uint32_t interval, uint32_t count,const char* filename, int32_t lineno) = 0;

    /*
    * @brief Timer���೤ʱ���ᴥ��
    * @param timer ���
    */
    virtual uint32_t GetTimeLeft(HTIMER timer) = 0;
    /*
    * @brief �رն�ʱ��
    */
    virtual void KillTimer(HTIMER timer) = 0;
    /*
    * @brief ��ѭ��������Ҫ��ͣ�ĵ���Update
    */
    virtual void Update() = 0;
    /*
    * @brief �ͷ�
    */
    virtual void Release() = 0;
};















// Kafka �� TimingWheel Դ�����ʱ���ֵĽ���
/*
 * Hierarchical Timing Wheels
 *
 * A simple timing wheel is a circular list of buckets of timer tasks. Let u be the time unit.
 * A timing wheel with size n has n buckets and can hold timer tasks in n * u time interval.
 * Each bucket holds timer tasks that fall into the corresponding time range. At the beginning,
 * the first bucket holds tasks for [0, u), the second bucket holds tasks for [u, 2u), ��,
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

