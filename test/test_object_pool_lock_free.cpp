#include "src/tools/object_pool_lock_free.h"
#include "unit_test_frame/unittest.h"
#include <stdio.h>
#include <vector>

FIXTURE_BEGIN(ObjectPoolLockFree)

// TODO 增加多线程测试用例 

/*
测试类
*/
class TestObjectPoolLockFree
{
public:
    TestObjectPoolLockFree(float x, float y, float z)
        : x_(x), y_(y), z_(z)
    {}
    void Print()
    {
        printf("x:%f,y:%f,z:%f\n", x_, y_, z_);
    }
private:
 float x_ = 0, y_ = 0, z_ = 0;
};

// 测试 取一个,归还一个
CASE(ObjectPoolLockFree1)
{
    // 开启打印
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().SetDebugPrint(false);
    // 打印对象池初始状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
    // 取一个 object
    auto* object = GetObjectLockFree<TestObjectPoolLockFree>(1.1,2.2,3.3);
    // 打印对象数据
    // object->Print();
    // 打印对象池状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
    // 归还对象
    GiveBackObjectLockFree<TestObjectPoolLockFree>(object);
    // 打印对象池状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
}

// 测试 [取一个,归还一个] 200次
CASE(ObjectPoolLockFree2)
{
    // 开启打印
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().SetDebugPrint(false);
    // 打印对象池初始状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
    for(int i = 0; i < 10000; i++)
    {
            // 取一个 object
            auto* object = GetObjectLockFree<TestObjectPoolLockFree>(1.1,2.2,3.3);
            // 打印对象数据
            // object->Print();
            // 打印对象池状态
            //GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
            // 归还对象
            GiveBackObjectLockFree<TestObjectPoolLockFree>(object);
    }
    // 打印对象池状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
}


// 测试 取200个,归还200个
CASE(ObjectPoolLockFree3)
{
    // 开启打印
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().SetDebugPrint(false);
    // 打印对象池初始状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
    std::vector<TestObjectPoolLockFree*> temp_object;
    for(int i = 0; i < 200; i++)
    {
            // 取一个 object
            auto* object = GetObjectLockFree<TestObjectPoolLockFree>(1.1,2.2,3.3);
            temp_object.emplace_back(object);
    }
    for(auto* object : temp_object)
    {
            // 归还对象
            GiveBackObjectLockFree<TestObjectPoolLockFree>(object);
    }
    temp_object.clear();
    // 打印对象池状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
}

// 测试 取 1000 个,归还 1000 个
CASE(ObjectPoolLockFree4)
{
    // 开启打印
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().SetDebugPrint(false);
    // 打印对象池初始状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
    std::vector<TestObjectPoolLockFree*> temp_object;
    for(int i = 0; i < 1000; i++)
    {
            // 取一个 object
            auto* object = GetObjectLockFree<TestObjectPoolLockFree>(1.1,2.2,3.3);
            temp_object.emplace_back(object);
    }
    for(auto* object : temp_object)
    {
            // 归还对象
            GiveBackObjectLockFree<TestObjectPoolLockFree>(object);
    }
    temp_object.clear();
    // 打印对象池状态
    GetObjectPoolLockFreeMgrRef<TestObjectPoolLockFree>().DebugPrint();
}

FIXTURE_END(ObjectPoolLockFree)