#pragma once

#include <list>
#include "fixture.h"
#include "singleton.h"

/*
* ��Ԫ���Կ��
*/
class UnitTest
{
public:
    /*
    * ����
    */
    UnitTest();
    /*
    * ����
    */
    ~UnitTest();
    /*
    * ��Ӳ�����������
    * @param  fixture ������������
    */
    bool AddFixture(Fixture* fixture);
    /*
    * ��ȡ������������
    * @param name ����������������
    */
    Fixture* GetFixture(const std::string& name);
    /*
    * ���ò������������Ƿ��ܹ�����
    * @param  name ����������������
    * @param  able �Ƿ�����
    */
    void SetFixtureRunable(const std::string name, bool able);
    /*
    * �������м���
    */
    void RunAllFixture();
    /*
    * ����һ�����Լ���
    * @param name ���Լ�������
    * @param count ���Լ������д���
    */
    void RunFixture(const std::string& name, std::size_t count = 1);
    /*
    * ��������˳�
    */
    void AnyKeyToQuit();
private:

    std::list<Fixture*> fixture_list_;
};

#define UnitTestMgr (*Singleton<UnitTest>::instance())