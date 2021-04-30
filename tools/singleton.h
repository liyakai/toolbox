# pragma once

#include <thread>
#include <atomic>
#include <mutex>

/*
*   懒汉模式创建单例
*/

template <typename ClassType>
class Singleton
{
public:
    /*
    *  获取单例
    *  @ param args 被实例化单例的构造参数
    *  @ return 单例指针
    */
    template<typename ...Args>
    static ClassType* instance(Args&&...args)
    {
        auto ins = instance_.load(std::memory_order_acquire);   // 限制多线程乱序,其它线程在std::memory_order_release之前的写操作在此之后均可见
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
    *  销毁单例实例
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
