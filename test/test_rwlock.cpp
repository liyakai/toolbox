#include "src/tools/rwlock.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestRWLock)

RWLock rw_lock;
CRWLock crw_lock;
constexpr int32_t target_num = 1000*1000;

void ReadThread_RWLock(int32_t& n)
{
    fprintf(stderr, "ReadThread_RWLock n:%d \n", n);
    while(n < target_num)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        rw_lock.ReadLock();
        fprintf(stderr, "ReadThread_RWLock read n:%d \n", n);
        rw_lock.ReadUnlock();
        std::this_thread::yield();
    }
}

void ReadThread_CRWLock(int32_t& n)
{
    fprintf(stderr, "ReadThread_CRWLock n:%d \n", n);
    while(n < target_num)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        fprintf(stderr, "before ReadThread_CRWLock read n:%d \n", n);
        crw_lock.ReadLock();
        fprintf(stderr, "ReadThread_CRWLock read n:%d \n", n);
        crw_lock.ReadUnlock();
        fprintf(stderr, "after ReadThread_CRWLock read n:%d \n\n", n);
        std::this_thread::yield();
    }
}

void WriteThread_RWLock(int32_t& n)
{
    while(n < target_num)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        rw_lock.WriteLock();
        fprintf(stderr, "WriteThread_RWLock change n:%d \n", ++n);
        rw_lock.WriteUnlock();
        //std::this_thread::yield();
    }     
}

void WriteThread_CRWLock(int32_t& n)
{
    while(n < target_num)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        fprintf(stderr, "before WriteThread_CRWLock change n:%d \n", n);
        crw_lock.WriteLock();
        fprintf(stderr, "WriteThread_CRWLock change n:%d \n", ++n);
        crw_lock.WriteUnlock();
        fprintf(stderr, "after WriteThread_CRWLock change n:%d \n\n", n);
        //std::this_thread::yield();
    }     
}

// 测试由互斥锁实现的读写锁
CASE(TestRWLockCase){
    int32_t n = 0;
    std::thread t1(ReadThread_RWLock, std::ref(n));
    // std::thread t2(ReadThread_RWLock, std::ref(n));
    std::thread t3(WriteThread_RWLock, std::ref(n));
    // std::thread t4(WriteThread_RWLock, std::ref(n));
    t1.join();
    //t2.join();
    t3.join();
    //t4.join();
    fprintf(stderr, "n:%d \n", n);
}

// 测试由互斥锁和条件变量实现的读写锁
CASE(CTestRWLockCase){
    return;
    int32_t n = 0;
    std::thread t1(ReadThread_CRWLock, std::ref(n));
    //std::thread t2(ReadThread_RWLock, std::ref(n));
    std::thread t3(WriteThread_CRWLock, std::ref(n));
    //std::thread t4(WriteThread_CRWLock, std::ref(n));
    t1.join();
    //t2.join();
    t3.join();
    //t4.join();
    fprintf(stderr, "n:%d \n", n);
}



FIXTURE_END(TestRWLock)