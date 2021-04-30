# pragma once

#include <thread>
#include <atomic>
#include <mutex>

/*
*   ����ģʽ��������
*/

template <typename ClassType>
class Singleton
{
public:
    /*
    *  ��ȡ����
    *  @ param args ��ʵ���������Ĺ������
    *  @ return ����ָ��
    */
    template<typename ...Args>
    static ClassType* instance(Args&&...args)
    {
        auto ins = instance_.load(std::memory_order_acquire);   // ���ƶ��߳�����,�����߳���std::memory_order_release֮ǰ��д�����ڴ�֮����ɼ�
        if(nullptr == ins)
        {
            std::lock_guard<std::mutex> lock(lock_);
            ins = instance_.load(std::memory_order_relaxed);
            if(nullptr == ins)
            {
                ins = new ClassType(std::forward<Args>(args)...);
                instance_.store(ins, std::memory_order_release);
            }
        }
        return ins;
    }

    /*
    *  ���ٵ���ʵ��
    */
   static void destroy()
   {
       auto ins = instance_.load(std::memory_order_acquire);
       if(null != ins)
       {
           std::lock_guard<std::mutex> lock(lock_);
           ins = instance_.load(std::memory_order_relaxed);
           delete ins;
           instance_.store(nullptr, std::memory_order_release);
       }
   }
private:
    Singleton() = delete;
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton<ClassType>& operator=(const Singleton&) = delete;



private:
    static std::mutex lock_;
    static std::atomic<ClassType*> instance_;
};

template <typename T>
std::mutex Singleton<T>::lock_;

template <typename T>
std::atomic<T*> Singleton<T>::instance_;
