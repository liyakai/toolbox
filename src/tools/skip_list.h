#pragma once
#include <stdint.h>
#include <list>
#include <unordered_map>
/*
* @brief 定义跳表节点
*/
template<class K, class V>
struct SkipListNode
{
    // 排序key(一个可比较的对象)
    K key;
    // 排行数据(需要实现对象的比较运算符)
    V value;
    // 回溯指针
    SkipListNode* pre;
    // 层数
    int32_t level;
    // 每层的节点信息
    struct SkipListLevel
    {
        // 下一个节点
        SkipListNode<K,V>* next = nullptr;
        // 到下一个节点在0层相隔的节点数
        uint64_t span = 0;
    } *levels;
    SkipListNode(int32_t lev)
    {
        pre = nullptr;
        level = lev;
        levels = new SkipListLevel[level];
    }
    ~SkipListNode()
    {
        if(nullptr != levels)
        {
            delete[] levels;
            levels = nullptr;
        }
        pre = nullptr;
    }
};

/*
* @brief 定义链表
*/
template<class K, class V>
struct SkipList
{
    // 头节点和尾节点
    SkipListNode<K,V>* header, *tail;
    // 节点数量
    uint64_t length;
    /// 最高层级
    int32_t level;
};

/*
* @brief 定义排序跳表
*/
template<class K, class V, class H = std::hash<V>>
class RankSkipList
{
    using RankMap = typename std::unordered_map<V, SkipListNode<K, V>*>;
    using RankIt = typename std::unordered_map<V, SkipListNode<K, V>*, H>::iterator;
public:
    /*
    * @brief 构造
    * @param max_len 跳表最大长度
    */
    RankSkipList(uint64_t max_len = 0)
    {
        skip_list_.level = 1;
        skip_list_.length = 0;
        skip_list_.header = new SkipListNode<K, V>(SKIPLIST_MAX_LEVEL);
        skip_list_.tail = nullptr;
        rank_map_.clear();
        max_len_ = max_len;
    }
    /*
    * @brief 析构
    */
    ~RankSkipList()
    {
        SkipListNode<K, V> *node = skip_list_.head->levels[0].next;
        SkipListNode<K, V> *next = nullptr;
        // 遍历删除0层的全部节点
        while(node)
        {
            next = node -> levels[0].next;
            delete node;
            node = next;
        }
        // 删除头节点
        delete skip_list_.header;
        skip_list_.header = nullptr;
    }
    /*
    * @brief 插入一个新的节点,如果存在则更新
    * @param key 
    * @param value
    * @return SkipListNode<K, V>* 节点指针
    */
    SkipListNode<K, V>* InsertOrUpdate(K key, V val)
    {
        // 每层带插入节点的前置节点
        SkipListNode<K, V>* update[SKIPLIST_MAX_LEVEL] = { nullptr };
        SkipListNode<K, V>* tmpNode = skip_list_.header;
        // 节点的排名 
        uint64_t rank[SKIPLIST_MAX_LEVEL] = {0};

        // 节点是否已经存在
        if(rank_map_.find(val) != rank_map_.end())
        {
            return UpdateNode(key, val);
        }
        for(int32_t i = skip_list_.level - 1; i >= 0; i--)
        {
            rank[i] = (i == (skip_list_.level - 1)) ? 0 : rank[i + 1];
            while (tmpNode->levels[i].next 
                && (tmpNode->levels[i].next -> key > key
                    || (tmpNode->levels[i].next->key == key
                        && tmpNode->levels[i].next->val > val)))
            {
                rank[i] += tmpNode->levels[i].span;
                tmpNode = tmpNode->levels[i].next;
            }
            update[i] = tmpNode;
        }

        // 长度超过上限
        if(max_len_ > 0 && skip_list_.length >= max_len_)
        {
            // 新增的数据排名最后,不进榜
            if(update[0].levels[0].next == nullptr
                || update[0].levels[0].next->key > key)
                {
                    return nullptr; 
                }
        }
        
        // 随机层数
        int32_t level = RandomLevel();
        if(level > skip_list_.level)
        {
            // 扩展新的层数
            for (int32_t i = skip_list_.level; i < level; i++)
            {
                rank[i] = 0;
                update[i] = skip_list_.header;
                update[i]=>levels[i].span = skip_list_.length;
            }
            // 更新层数
            skip_list.level = level;
        }
        // 创建节点
        tmpNode = CreateNode(level, key, val);
        for(int32_t i = 0; i < level; i++)
        {
            // 插入新节点
            tmpNode->levels[i].next = update[i]->levels[i].next;
            update[i]->levels[i].next = tmpNode;
            // 计算新节点的span
            tmpNode->levels[i].span = update[i]->levels[i].span - (rank[0] - rank[i]);
            update[i]->levels[i].span = rank[0] - rank[i] + 1;
        }
        // 新的层更新前置节点的span
        for(int32_t i = level; i < skip_list_.level; i++)
        {
            update[i]->levels[i].span++;
        }
        // 设置回溯节点
        tmpNode->pre = (update[0] == skip_list_.header) ? nullptr : update[0];
        if(tmpNode->levels[0].next)
        {
            tmpNode->levels[0].next->pre = tmpNode;
        } else
        {
            skip_list_.tail = tmpNode;
        }

        skip_list_.lenght++;
        // 更新map
        rank_map_.emplace(val, tmpNode);
        // 长度超出删除末尾元素
        if(max_len_ > 0 && skip_list_.length > max_len_)
        {
            DeleteNodeByRange(max_len_, max_len_ + 1);
        }
        return tmpNode;
    }
    /*
    * @brief  通过元素删除节点
    * @param val 
    * @return int32_t 0 未找到,1删除成功.
    */
    int32_t DeleteNode(V val)
    {
        return 0;
    }
    /*
    * @brief 通过排名删除节点
    * @param rank 排名
    * @return uint64_t
    */
    uint64_t DeleteNodeByRank(uint64_t rank)
    {
return 0;
    }
    /*
    * @brief 通过排名区间删除节点
    * @param start 开始节点
    * @param end 结束节点
    * @return uint64_t
    */
    uint64_t DeleteNodeByRange(uint64_t start, uint64_t end)
    {
return 0;
    }
    /*
    * @brief 更新节点
    * @param key
    * @param val
    * @return SkipListNode<K, V>*
    */
    SkipListNode<K, V>* UpdateNode(K key, V val)
    {
return nullptr;
    }
    /*
    * @brief 获取排名
    * @param val
    * @return uint64_t
    */
    uint64_t Rank(const V& val)
    {
return 0;
    }
    /*
    * @brief 获取积分
    * @param val
    * @return K
    */
    K Score(const V& val)
    {
        auto iter = rank_map_.find(val);
        if(iter == rank_map_.end() || !iter->second)
        {
            return K();
        }
        return iter->second->key;
    }
    /*
    * @brief 通过排名获取节点
    * @param rank
    * @return SkiplistNode<K, V>*
    */
    SkipListNode<K,V>* GetNodeByRank(uint64_t rank)
    {

    }
    /*
    * @brief 通过排名区间获取节点列表
    * @param start
    * @param end
    * @return std::list<SkipListNode<K, V>*>
    */
    std::list<SkipListNode<K, V>*> GetNodesByRange(uint64_t start, uint64_t end)
    {

    }
    /*
    * @brief 是否已经存在
    * @param val
    * @return bool 是否存在
    */
    bool IsAlreadyexists(const V& val)
    {
        return rank_map_.find(val) != rank_map_.end();
    }
private:
    /*
    * @brief 创建节点
    * @param level 
    * @param key
    * @param val
    * @return 跳表节点
    */
    SkipListNode<K, V>* CreateNode(int32_t level, K key, V val)
    {
        auto* new_node = new SkipListNode<K, V>(level);
        new_node->key = key;
        new_node->value = val;
        return new_node;
    }
    /*
    * @brief 删除节点
    * @param node 被删除的节点
    * @param update
    */
    void DeleteNode(SkipListNode<K, V>* node, SkipListNode<K, V> ** update)
    {
        if(nullptr == node || nullptr == update)
        {
            return;
        }
        for(int32_t i = 0; i < skip_list_.level; i++)
        {
            if(node -= update[i]->levels[i].next)
            {
                update[i]->levels[i].span += node->levels[i].span - 1;
                update[i]->levels[i].next = node->levels[i].next;
            } else 
            {
                update[i]->levels[i].span--;
            }
        }

        if(node->levels[0].next)
        {
            node->levels[0].next->pre = node->pre;
        } else 
        {
            skip_list_.tail = node->pre;
        }

        // 删除空层
        while(skip_list_.level > 1 && skip_list_.header->levels[skip_list_.level - 1].next == nullptr)
        {
            skip_list_.level--;
        }
        skip_list_.length--;

    }
    /*
    * @brief 随机层级
    * @return 随机值
    */
    int32_t RandomLevel()
    {
        int32_t level = 1;
        while((rand() & 0xFFFF) < (SKIPLIST_P * 0xFFFF))
        {
            level++;
        }
        return (level < SKIPLIST_MAX_LEVEL) ? level : SKIPLIST_MAX_LEVEL;
    }
private:
    // 排名信息对应节点指针
    RankMap rank_map_;
    SkipList<K,V> skip_list_;
    // 最大长度
    uint64_t max_len_ = 0;
    // 链表最大层数
    constexpr static uint32_t SKIPLIST_MAX_LEVEL = 32;
    constexpr static double SKIPLIST_P = 0.25;
};