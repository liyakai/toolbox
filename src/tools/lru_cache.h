#pragma once
#include <algorithm>
#include <unordered_map>
#include <list>
#include <mutex>


/*
* 定义一个无锁之锁.[仿造互斥锁 std::mutex]
*/
class NullLock
{
public:
    void lock(){}
    void unlock(){}
    bool try_lock(){ return true; }
};

/*
* 定义 KeyValue 对
*/
template<typename K, typename V>
struct KeyValuePair
{
public:
    K key;
    V value;

    KeyValuePair(K k, V v) 
        : key(std::move(k)), value(std::move(v))
    {}
};

/*
* 定义模板 LRU Cache
* Key key类型
* Value value类型
* Lock 锁类型,默认无锁.
* MapType 一个关联式容器,比如 std::unordered_map
* 默认的锁类型 NullLock 不是线程安全的,如果传递一个互斥锁[e.g. Lock=std::mutex]，则会变为线程安全的.
*/
template <typename Key, typename Value, typename Lock = NullLock, 
          typename Map = std::unordered_map<Key, typename std::list<KeyValuePair<Key,Value>>::iterator>>
class LRUCache
{
public:
    typedef KeyValuePair<Key,Value> node_type;
    typedef std::list<KeyValuePair<Key, Value>> list_type;
    typedef Map map_type;
    typedef Lock lock_type;
    using Guard = std::lock_guard<lock_type>;

    /*
    * @brief 构造
    */
    explicit LRUCache(std::size_t max_size = 64, std::size_t elasticity = 10)
        : max_size_(max_size), elasticity_(elasticity){}
    /*
    * @brief 析构
    */
    virtual ~LRUCache() = default;
    /*
    * @brief 缓存大小
    */
    std::size_t Size() const 
    {
        Guard g(lock_);
        return cache_.size();
    }
    /*
    * @brief 缓存是否为空
    */
    bool Empty()
    {
        Guard g(lock_);
        return cache_.size();
    }
    /*
    * @brief 清空
    */
    void Clear()
    {
        Guard g(lock_);
        cache_.clear();
        keys_.clear();
    }
    /*
    * @brief 插入
    */
    void Insert(const Key& k, Value v)
    {
        Guard g(lock_);
        const auto iter = cache_.find(k);
        if(iter != cache_.end())
        {
            iter->second->value = v;
            keys_.splice(keys_.begin(), keys_, iter->second);
            return;
        }
        keys_.emplace_front(k, std::move(v));
        cache_[k] = keys_.begin();
        Prune();
    }
    /*
    * @brief 获取
    */
    bool Get(const Key& key_in, Value& value_out)
    {
        Guard g(lock_);
        const auto iter = cache_.find(key_in);
        if(iter == cache_.end())
        {
            return false;
        }
        keys_.splice(keys_.begin(), keys_, iter->second);
        value_out = iter->second->value;
        return true;
    }
    /*
    * @brief 删除
    */
    bool Remove(const Key& key)
    {
        Guard g(lock_);
        auto iter = cache_.find(key);
        if(iter == cache_.end())
        {
            return false;
        }
        keys_.erase(iter->second);
        cache_.erase(iter);
        return true;
    }
    /*
    * @breif 是否包含元素
    */
    bool Contains(const Key& key) const
    {
        Guard g(lock_);
        return cache_.find(key) != cache_.end();
    }
    /*
    * @brief 遍历
    */
    template<typename Func>
    void Foreach(Func& func) const
    {
        Guard g(lock_);
        std::for_each(keys_.begin(), keys_.end(), func);
    }
    /*
    * @brief 获取最大值
    */
    std::size_t GetMaxSize() const { return max_size_; }
    /*
    * @brief 获取弹性值
    */
    std::size_t GetElasticity() const { return elasticity_; }
    /*
    * @brief 获取允许的最大值
    */
    std::size_t GetMaxAllowedSize() const { return max_size_ + elasticity_; }
protected:
    /*
    * @brief 修剪,删除超出的数据
    */
    std::size_t Prune()
    {
        std::size_t max_allowed = max_size_ + elasticity_;
        if(0 == max_size_ || cache_.size() < max_allowed)
        {
            return 0;
        }
        std::size_t count = 0;
        while(cache_.size() > max_size_)
        {
            cache_.erase(keys_.back().key);
            keys_.pop_back();
            count++;
        }
        return count;
    }

private:
    // 禁止拷贝构造与赋值
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    mutable Lock lock_;
    Map cache_;
    list_type keys_;
    std::size_t max_size_ = 0;
    std::size_t elasticity_ = 0;
};