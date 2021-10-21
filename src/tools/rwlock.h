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
* @brief 使用互斥锁和条件变量实现读写锁 [没有通过单元测试,切勿使用!!!!!!]
*/
class CRWLock
{
public:
    /*
    * 构造
    */
    CRWLock()
        :m_cnt_(0)
    {

    }
    /*
    * @brief 添加读锁
    */
    void ReadLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        if(m_cnt_ < 0)
        {
            cond_var_.wait(lck);
        }
        fprintf(stderr, "ReadLock m_cnt_:%d\n", m_cnt_.load());
        m_cnt_++;
        fprintf(stderr, "ReadLock m_cnt_:%d\n", m_cnt_.load());
    }
    /*
    * @brief 解除读锁
    */
    void ReadUnlock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        if(--m_cnt_ == 0)
        {
            fprintf(stderr, "ReadUnlock notify_one m_cnt_:%d\n", m_cnt_.load());
            cond_var_.notify_one();
        }
        fprintf(stderr, "ReadUnlock m_cnt_:%d\n", m_cnt_.load());
    }
    /*
    * @brief 添加写锁
    */
    void WriteLock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        
        while(m_cnt_ != 0)
        {
            fprintf(stderr, "WriteLock wait m_cnt_:%d\n", m_cnt_.load());
            cond_var_.wait(lck);
        }
        m_cnt_ = -1;
        fprintf(stderr, "WriteLock m_cnt_:%d  [-1]\n", m_cnt_.load());
    }
    /*
    * @brief 解除写锁
    */
    void WriteUnlock()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        m_cnt_ = 0;
        fprintf(stderr, "WriteUnlock m_cnt_:%d\n", m_cnt_.load());
        cond_var_.notify_all();                 // 叫醒所有等待的读和写操作
    }
private:
    std::mutex mtx_;                            // 保护计数资源
    std::condition_variable cond_var_;          // 条件变量
    std::atomic<int32_t> m_cnt_;                // 计数, ==0无读写, >0读数量, <0在写
};