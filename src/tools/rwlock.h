/*--------------------------------------------------------------------
*FileName:			rwlock.h
*Description :		两种读写锁的实现
*Author :			liyakai
*Date :				2021.10.17
*Copyright(C):
--------------------------------------------------------------------*/
# pragma once
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace ToolBox{
/*
* 仅使用互斥锁实现读写锁
*/
class RWLock
{
public:
    /*
    * @brief 添加读锁
    */
    void ReadLock()
    {
        std::lock_guard<std::mutex> lck(read_mtx_);
        if(++read_cnt_ == 1)
        {
            write_mtx_.lock();
        }
    }
    /*
    * @brief 解除读锁
    */
    void ReadUnlock()
    {
        std::lock_guard<std::mutex> lck(read_mtx_);
        if(--read_cnt_ == 0)
        {
            write_mtx_.unlock();
        }
    }
    /*
    * @brief 添加写锁
    */
    void WriteLock()
    {
        write_mtx_.lock();
    }
    /*
    * @brief 解除写锁
    */
    void WriteUnlock()
    {
        write_mtx_.unlock();
    }
private:
    std::mutex read_mtx_;
    std::mutex write_mtx_;
    volatile int32_t read_cnt_ = 0;
};

/*
* @brief 使用互斥锁和条件变量实现读写锁 [经测试,此实现有问题,禁止使用!!]
*/
class CRWLock
{
public:
    /*
    * 构造
    */
    CRWLock()
        :read_cnt_(0)
    {

    }
    /*
    * @brief 添加读锁
    */
    void ReadLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        cond_var_.wait(lck, std::bind(&CRWLock::IsReadCond, this));
        read_cnt_++;
    }
    /*
    * @brief 尝试获取读锁
    */
    bool TryReadLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        if(cond_var_.wait_for(lck, std::chrono::seconds(0)) == std::cv_status::timeout)
        {
            return false;
        }
        if(IsReadCond())
        {
            read_cnt_++;
            return true;
        }
        return false;
    }
    /*
    * @brief 解除读锁
    */
    void ReadUnlock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        read_cnt_--;
        cond_var_.notify_all();
    }
    /*
    * @brief 添加写锁
    */
    void WriteLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        cond_var_.wait(lck, std::bind(&CRWLock::IsWriteCond, this));
        is_write_ = true;
    }
        /*
    * @brief 尝试获取写锁
    */
    bool TryWriteLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        if(cond_var_.wait_for(lck, std::chrono::seconds(0)) == std::cv_status::timeout)
        {
            return false;
        }
        if(IsWriteCond())
        {
            is_write_ = true;
            return true;
        }
        return false;
    }
    /*
    * @brief 解除写锁
    */
    void WriteUnlock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        is_write_ = false;
        cond_var_.notify_all();                 // 叫醒所有等待的读和写操作
    }
private:
    /*
    * @brief 读条件
    */
    bool IsReadCond() const
    {
        return false == is_write_;
    }
    /*
    * @brief 写条件
    */
    bool IsWriteCond() const
    {
        return false == is_write_ && 0 == read_cnt_;
    }
private:
    std::mutex mtx_;                            // 保护计数资源
    std::condition_variable cond_var_;          // 条件变量
    std::size_t read_cnt_;                      // 读者数量
    bool is_write_ = false;                     // 是否在写
};



};  // ToolBox