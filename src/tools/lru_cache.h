#pragma once
#include <algorithm>
#include <unordered_map>
#include <list>
#include <mutex>


/*
* ����һ������֮��.[���컥���� std::mutex]
*/
class NullLock
{
public:
    void lock(){}
    void unlock(){}
    bool try_lock(){ return true; }
};

/*
* ���� KeyValue ��
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
* ����ģ�� LRU Cache
* Key key����
* Value value����
* Lock ������,Ĭ������.
* MapType һ������ʽ����,���� std::unordered_map
* Ĭ�ϵ������� NullLock �����̰߳�ȫ��,�������һ��������[e.g. Lock=std::mutex]������Ϊ�̰߳�ȫ��.
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
    * @brief ����
    */
    explicit LRUCache(std::size_t max_size = 64, std::size_t elasticity = 10)
        : max_size_(max_size), elasticity_(elasticity){}
    /*
    * @brief ����
    */
    virtual ~LRUCache() = default;
    /*
    * @brief �����С
    */
    std::size_t Size() const 
    {
        Guard g(lock_);
        return cache_.size();
    }
    /*
    * @brief �����Ƿ�Ϊ��
    */
    bool Empty()
    {
        Guard g(lock_);
        return cache_.size();
    }
    /*
    * @brief ���
    */
    void Clear()
    {
        Guard g(lock_);
        cache_.clear();
        keys_.clear();
    }
    /*
    * @brief ����
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
    * @brief ��ȡ
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
    * @brief ɾ��
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
    * @breif �Ƿ����Ԫ��
    */
    bool Contains(const Key& key) const
    {
        Guard g(lock_);
        return cache_.find(key) != cache_.end();
    }
    /*
    * @brief ����
    */
    template<typename Func>
    void Foreach(Func& func) const
    {
        Guard g(lock_);
        std::for_each(keys_.begin(), keys_.end(), func);
    }
    /*
    * @brief ��ȡ���ֵ
    */
    std::size_t GetMaxSize() const { return max_size_; }
    /*
    * @brief ��ȡ����ֵ
    */
    std::size_t GetElasticity() const { return elasticity_; }
    /*
    * @brief ��ȡ��������ֵ
    */
    std::size_t GetMaxAllowedSize() const { return max_size_ + elasticity_; }
protected:
    /*
    * @brief �޼�,ɾ������������
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
    // ��ֹ���������븳ֵ
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    mutable Lock lock_;
    Map cache_;
    list_type keys_;
    std::size_t max_size_ = 0;
    std::size_t elasticity_ = 0;
};