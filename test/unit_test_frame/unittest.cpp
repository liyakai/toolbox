#include "unittest.h"
#include<iostream>

UnitTest::UnitTest()
{

}

UnitTest::~UnitTest()
{
    for (auto fixture : fixture_list_)
    {
        delete fixture;
    }
}

bool UnitTest::AddFixture(Fixture* fixture)
{
    if (nullptr == fixture)
    {
        return false;
    }
    fixture_list_.push_back(fixture);
    return true;
}

Fixture* UnitTest::GetFixture(const std::string& name)
{
    for  (auto fixture : fixture_list_)
    {
        if (fixture && fixture->GetName() == name)
        {
            return fixture;
        }
    }
    return nullptr;
}

void UnitTest::SetFixtureRunable(const std::string name, bool able)
{
    for (auto fixture : fixture_list_)
    {
        if (fixture && fixture->GetName() == name)
        {
            fixture->SetRunable(able);
        }
    }
}

void UnitTest::RunAllFixture()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    system("chcp 65001");   // 让windows控制台以 UTF-8显示
#elif defined(__linux__)
#endif  // #if (defined(WIN32) || defined(_WIN64))
    for (auto fixture : fixture_list_)
    {
        if (fixture && fixture->IsEnable())
        {
            fixture->Init();
            fixture->RunAllTestCase();
            fixture->UnInit();
        }
    }
}

void UnitTest::RunFixture(const std::string& name, std::size_t count /*= 1*/)
{
    for (auto fixture : fixture_list_)
    {
        if (fixture && fixture->GetName() == name)
        {
            for (std::size_t i = 0; i < count; i++)
            {
                fixture->RunAllTestCase();
            }
        }
    }
}

void UnitTest::AnyKeyToQuit()
{
    printf("Press any key to quit.\n");
    std::getc(stdin);
}

