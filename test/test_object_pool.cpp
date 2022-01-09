#include "src/tools/object_pool.h"
#include "unit_test_frame/unittest.h"
#include <stdio.h>
#include <vector>

FIXTURE_BEGIN(ObjectPool)

/*
测试类
*/
class TestObjectPool
{
public:
    TestObjectPool(float x, float y, float z)
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
CASE(ObjectPool1)
{
    // 开启打印
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().SetDebugPrint(false);
    // 打印对象池初始状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
    // 取一个 object
    auto* object = ToolBox::GetObject<TestObjectPool>(1.1,2.2,3.3);
    // 打印对象数据
    // object->Print();
    // 打印对象池状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
    // 归还对象
    ToolBox::GiveBackObject<TestObjectPool>(object);
    // 打印对象池状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
}

// 测试 [取一个,归还一个] 200次
CASE(ObjectPool2)
{
    // 开启打印
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().SetDebugPrint(false);
    // 打印对象池初始状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
    for(int i = 0; i < 100; i++)
    {
            // 取一个 object
            auto* object = ToolBox::GetObject<TestObjectPool>(1.1,2.2,3.3);
            // 打印对象数据
            // object->Print();
            // 打印对象池状态
            //GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
            // 归还对象
            ToolBox::GiveBackObject<TestObjectPool>(object);
    }
    // 打印对象池状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
}


// 测试 取200个,归还200个
CASE(ObjectPool3)
{
    // 开启打印
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().SetDebugPrint(false);
    // 打印对象池初始状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
    std::vector<TestObjectPool*> temp_object;
    for(int i = 0; i < 200; i++)
    {
            // 取一个 object
            auto* object = ToolBox::GetObject<TestObjectPool>(1.1,2.2,3.3);
            temp_object.emplace_back(object);
    }
    for(auto* object : temp_object)
    {
            // 归还对象
            ToolBox::GiveBackObject<TestObjectPool>(object);
    }
    temp_object.clear();
    // 打印对象池状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
}

// 测试 取 1000 个,归还 1000 个
CASE(ObjectPool4)
{
    // 开启打印
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().SetDebugPrint(false);
    // 打印对象池初始状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
    std::vector<TestObjectPool*> temp_object;
    for(int i = 0; i < 1000; i++)
    {
            // 取一个 object
            auto* object = ToolBox::GetObject<TestObjectPool>(1.1,2.2,3.3);
            temp_object.emplace_back(object);
    }
    for(auto* object : temp_object)
    {
            // 归还对象
            ToolBox::GiveBackObject<TestObjectPool>(object);
    }
    temp_object.clear();
    // 打印对象池状态
    ToolBox::GetObjectPoolMgrRef<TestObjectPool>().DebugPrint();
}

FIXTURE_END(ObjectPool)