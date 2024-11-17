#include "tools/function_name.h"
#include "unit_test_frame/unittest.h"
#include <thread>
#ifdef __linux__

FIXTURE_BEGIN(FunctionName)

void NormalFunction()
{
    fprintf(stderr, "printNormalFunction \n");
}

// 获取普通函数名字
CASE(TestFunctionNameCase1)
{
    auto name = GetFuncName<NormalFunction>();
    fprintf(stderr, "Get normal function name: %s\n", name.data());
}

class TestClass
{
public:
    void TestClassMethod();
};

// 获取类成员函数名字
CASE(TestClassFunctionNameCase2)
{
    auto name = GetFuncName<&TestClass::TestClassMethod>();
    fprintf(stderr, "Get class method name: %s\n", name.data());

}

// 获取 lambda 表达式名字
CASE(TestLambdaFunctionNameCase3)
{
    auto lambda = []() { return 1; };
    auto name = GetFuncName<lambda>();
    fprintf(stderr, "Get lambda function name: %s\n", name.data());
}

FIXTURE_END(FunctionName)

#endif // __linux__