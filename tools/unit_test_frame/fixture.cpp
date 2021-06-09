#include "fixture.h"
#include <stdio.h>
#include <algorithm>

Fixture::Fixture(const std::string& name)
    : name_(name), run_(true)
{

}

Fixture::~Fixture()
{
    for (auto test_case : case_list_)
    {
        delete test_case;
    }
}

void Fixture::Init()
{
    if (init_)
    {
        init_();
    }
}

void Fixture::UnInit()
{
    if (uninit_)
    {
        uninit_();
    }
}

bool Fixture::SetInitFunc(FixtureInitMethod method)
{
    init_ = method;
    return true;
}

bool Fixture::SetUninitFunc(FixtureUninitMethod method)
{
    uninit_ = method;
    return true;
}

const std::string& Fixture::GetName() const
{
    return name_;
}

bool Fixture::AddTestCase(TestCase* testcase)
{
    case_list_.push_back(testcase);
    return true;
}

TestCase* Fixture::GetTestCase(const std::string& name)
{
    for (auto testcase : case_list_)
    {
        if (testcase->GetName() == name)
        {
            return testcase;
        }
    }
    return nullptr;
}

void Fixture::SetTestCase(const std::string& name, bool able)
{
    auto iter = std::find_if(case_list_.begin(), case_list_.end(), [&](const TestCase* testcase) {
        return testcase->GetName() == name;
        });
    if (iter == case_list_.end())
    {
        return;
    }
    (*iter)->SetNeedRun(able);
}

void Fixture::RunAllTestCase()
{
    printf("运行测试集合[ %s ]\n",name_.c_str());
    std::size_t pass = 0;
    std::size_t fail = 0;
    std::size_t disable_count = 0;
    std::size_t exception_count = 0;
    for (auto testcase : case_list_)
    {
        if (testcase->IsEnable())
        {
            printf("运行 [ %s ] ", testcase->GetName().c_str());
            try
            {
                testcase->Run();
                if (testcase->IsSuccess())
                {
                    pass++;
                    printf("[ PASS ]\n");
                }
                else
                {
                    fail++;
                    printf("[ FAIL ]\n");
                    for (auto& error : testcase->GetErrorList())
                    {
                        printf(" > %s\n", error.c_str());
                    }
                }
            }
            catch (std::exception& e)
            {
                fail++;
                exception_count++;
                printf("[ EXCEPTION ]\n");
                printf("[ %s ]\n", typeid(e).name());
                printf("[ %s ]\n", e.what());
            }
            catch (...)
            {
                fail++;
                exception_count++;
                printf("[ EXCEPTION ]\n");
            }
            testcase->ClearError();
        }
        else
        {
            disable_count++;
            printf("测试用例[ %s ] [DISABLED]\n", testcase->GetName().c_str());
        }
    }
    printf("\n结果统计:\n");
    printf("[ %zu ][ PASS ]\n", pass);
    printf("[ %zu ][ FAIL ]\n", fail);
    printf("[ %zu ][ EXCEPTION ]\n", exception_count);
    printf("[ %zu ][ DISABLE ]\n", disable_count);
    printf("\n\n");
}

bool Fixture::IsEnable()
{
    return run_;
}

void Fixture::SetRunable(bool able)
{
    run_ = able;
}

