#pragma once

#include <list>
#include "fixture.h"
#include "singleton.h"

/*
* 单元测试框架
*/
class UnitTest
{
public:
    /*
    * 构造
    */
    UnitTest();
    /*
    * 析构
    */
    ~UnitTest();
    /*
    * 添加测试用例集合
    * @param  fixture 测试用例集合
    */
    bool AddFixture(Fixture* fixture);
    /*
    * 获取测试用例集合
    * @param name 测试用例集合名称
    */
    Fixture* GetFixture(const std::string& name);
    /*
    * 设置测试用例集合是否能够运行
    * @param  name 测试用例集合名字
    * @param  able 是否运行
    */
    void SetFixtureRunable(const std::string name, bool able);
    /*
    * 运行所有集合
    */
    void RunAllFixture();
    /*
    * 运行一个测试集合
    * @param name 测试集合名字
    * @param count 测试集合运行次数
    */
    void RunFixture(const std::string& name, std::size_t count = 1);
    /*
    * 按任意键退出
    */
    void AnyKeyToQuit();
private:

    std::list<Fixture*> fixture_list_;
};

#define UnitTestMgr (*Singleton<UnitTest>::instance())
