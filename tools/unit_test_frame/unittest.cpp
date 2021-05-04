#include "unittest.h"

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
    scanf("\t");
}

