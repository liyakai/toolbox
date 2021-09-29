#pragma once

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
    typedef struct SkipListLevel
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
    SkipListNode<K,V>* header, *tai;
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
    using RankIt = typename std::unordered_map<V, SkipListNode<K, V>*, H>::iterator;
};