#include "tools/function_name.h"
#include "unit_test_frame/unittest.h"
#ifdef __linux__

FIXTURE_BEGIN(FunctionName)

void NormalFunction()
{
    fprintf(stderr, "printNormalFunction \n");
}

// 获取普通函数名字
CASE(TestFunctionNameCase1)
{
    auto name = ToolBox::GetFuncName<NormalFunction>();
    fprintf(stderr, "Get normal function name: %s\n", name.data());
}

class TestClass
{
public:
    void TestClassMethod() {}
    void TestClassMethod(int x) { (void)x; }
    static void StaticMethod() {}
    void ConstMethod() const {}
    int GetValue() const { return 42; }
};

// 获取类成员函数名字
CASE(TestClassFunctionNameCase2)
{
    // 使用无参数版本的重载函数，需要明确类型
    constexpr void (TestClass::*method_ptr)() = &TestClass::TestClassMethod;
    auto name = ToolBox::GetFuncName<method_ptr>();
    fprintf(stderr, "Get class method name: %s\n", name.data());
}

// 测试 GetFuncNameWithNamespace - 普通函数
CASE(TestGetFuncNameWithNamespace)
{
    auto name = ToolBox::GetFuncNameWithNamespace<NormalFunction>();
    fprintf(stderr, "Get function name with namespace: %.*s\n", static_cast<int>(name.size()), name.data());
}

// 测试 GetFuncNameWithNamespace - 类成员函数
CASE(TestGetFuncNameWithNamespaceClassMethod)
{
    constexpr void (TestClass::*method_ptr)() = &TestClass::TestClassMethod;
    auto name = ToolBox::GetFuncNameWithNamespace<method_ptr>();
    fprintf(stderr, "Get class method name with namespace: %.*s\n", static_cast<int>(name.size()), name.data());
}

// 测试 GetFuncNameWithClass - 类成员函数
CASE(TestGetFuncNameWithClass)
{
    constexpr void (TestClass::*method_ptr)() = &TestClass::TestClassMethod;
    auto name = ToolBox::GetFuncNameWithClass<method_ptr>();
    fprintf(stderr, "Get class method name with class: %.*s\n", static_cast<int>(name.size()), name.data());
}

// 测试 GetFuncNameWithClass - 普通函数（应该返回函数名）
CASE(TestGetFuncNameWithClassNormalFunction)
{
    auto name = ToolBox::GetFuncNameWithClass<NormalFunction>();
    fprintf(stderr, "Get normal function name with class: %.*s\n", static_cast<int>(name.size()), name.data());
}

// 测试静态成员函数
CASE(TestStaticMethod)
{
    auto name = ToolBox::GetFuncName<&TestClass::StaticMethod>();
    fprintf(stderr, "Get static method name: %s\n", name.data());
    
    auto name_with_class = ToolBox::GetFuncNameWithClass<&TestClass::StaticMethod>();
    fprintf(stderr, "Get static method name with class: %.*s\n", static_cast<int>(name_with_class.size()), name_with_class.data());
}

// 测试 const 成员函数
CASE(TestConstMethod)
{
    auto name = ToolBox::GetFuncName<&TestClass::ConstMethod>();
    fprintf(stderr, "Get const method name: %s\n", name.data());
    
    auto name_with_class = ToolBox::GetFuncNameWithClass<&TestClass::ConstMethod>();
    fprintf(stderr, "Get const method name with class: %.*s\n", static_cast<int>(name_with_class.size()), name_with_class.data());
}

// 测试带返回值的成员函数
CASE(TestMethodWithReturnValue)
{
    auto name = ToolBox::GetFuncName<&TestClass::GetValue>();
    fprintf(stderr, "Get method with return value name: %s\n", name.data());
    
    auto name_with_class = ToolBox::GetFuncNameWithClass<&TestClass::GetValue>();
    fprintf(stderr, "Get method with return value name with class: %.*s\n", static_cast<int>(name_with_class.size()), name_with_class.data());
}

// 测试重载函数
CASE(TestOverloadedMethod)
{
    // 测试无参数版本
    constexpr void (TestClass::*method_ptr)() = &TestClass::TestClassMethod;
    auto name = ToolBox::GetFuncName<method_ptr>();
    fprintf(stderr, "Get overloaded method name (no params): %s\n", name.data());
    
    // 测试带参数版本
    constexpr void (TestClass::*overloaded_ptr)(int) = &TestClass::TestClassMethod;
    auto name2 = ToolBox::GetFuncName<overloaded_ptr>();
    fprintf(stderr, "Get overloaded method name (with int param): %s\n", name2.data());
}

// 测试命名空间中的函数
namespace TestNamespace {
    void NamespaceFunction() {}
    class NamespaceClass {
    public:
        void NamespaceMethod() {}
    };
}

CASE(TestNamespaceFunction)
{
    auto name = ToolBox::GetFuncName<TestNamespace::NamespaceFunction>();
    fprintf(stderr, "Get namespace function name: %s\n", name.data());
    
    auto name_with_namespace = ToolBox::GetFuncNameWithNamespace<TestNamespace::NamespaceFunction>();
    fprintf(stderr, "Get namespace function name with namespace: %.*s\n", static_cast<int>(name_with_namespace.size()), name_with_namespace.data());
}

CASE(TestNamespaceClassMethod)
{
    auto name = ToolBox::GetFuncName<&TestNamespace::NamespaceClass::NamespaceMethod>();
    fprintf(stderr, "Get namespace class method name: %s\n", name.data());
    
    auto name_with_class = ToolBox::GetFuncNameWithClass<&TestNamespace::NamespaceClass::NamespaceMethod>();
    fprintf(stderr, "Get namespace class method name with class: %.*s\n", static_cast<int>(name_with_class.size()), name_with_class.data());
}

// 测试继承类中的函数
class BaseClass {
public:
    virtual void VirtualMethod() {}
    void BaseMethod() {}
};

class DerivedClass : public BaseClass {
public:
    void DerivedMethod() {}
    void BaseMethod() {} // 隐藏基类方法
};

CASE(TestInheritedMethod)
{
    auto base_name = ToolBox::GetFuncName<&BaseClass::BaseMethod>();
    fprintf(stderr, "Get base class method name: %s\n", base_name.data());
    
    auto derived_name = ToolBox::GetFuncName<&DerivedClass::DerivedMethod>();
    fprintf(stderr, "Get derived class method name: %s\n", derived_name.data());
    
    auto virtual_name = ToolBox::GetFuncName<&BaseClass::VirtualMethod>();
    fprintf(stderr, "Get virtual method name: %s\n", virtual_name.data());
    
    auto derived_base_name = ToolBox::GetFuncName<&DerivedClass::BaseMethod>();
    fprintf(stderr, "Get derived class base method name: %s\n", derived_base_name.data());
}

// 获取 lambda 表达式名字
CASE(TestLambdaFunctionNameCase3)
{
    auto lambda = []() { return 1; };
    auto name = ToolBox::GetFuncName<lambda>();
    fprintf(stderr, "Get lambda function name: %s\n", name.data());
    
    auto name_with_namespace = ToolBox::GetFuncNameWithNamespace<lambda>();
    fprintf(stderr, "Get lambda function name with namespace: %.*s\n", static_cast<int>(name_with_namespace.size()), name_with_namespace.data());
}

// 测试带参数的 lambda
CASE(TestLambdaWithParams)
{
    auto lambda = [](int x, int y) { return x + y; };
    auto name = ToolBox::GetFuncName<lambda>();
    fprintf(stderr, "Get lambda with params name: %s\n", name.data());
}

FIXTURE_END(FunctionName)

#endif // __linux__