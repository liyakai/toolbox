#include <iostream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <functional>
#include "../include/tools/platform.h"
#include "../include/tools/safe_call.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestSafeCall)

// ========== 测试辅助函数 ==========

// 正常函数：返回整数
int NormalFunction(int x) {
    return x * 2;
}

// 正常函数：void 返回类型
void NormalVoidFunction(int& value) {
    value = 42;
}

// 抛出 std::exception 的函数
int ThrowStdException(int x) {
    throw std::runtime_error("测试运行时错误");
    return x;
}

// 抛出非标准异常的函数
int ThrowUnknownException(int x) {
    throw "未知异常";
    return x;
}

// void 函数抛出异常
void VoidThrowException() {
    throw std::logic_error("void 函数异常测试");
}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
// 触发 SIGSEGV 的函数
int TriggerSIGSEGV() {
    int* ptr = nullptr;
    return *ptr;  // 空指针解引用
}

// 触发 SIGFPE 的函数
int TriggerSIGFPE() {
    volatile int a = 10;
    volatile int b = 0;
    return a / b;  // 除零错误
}

// void 函数触发信号
void VoidTriggerSIGSEGV() {
    int* ptr = nullptr;
    *ptr = 42;  // 空指针写入
}
#endif

// 测试类
class TestClass {
public:
    int value;
    
    TestClass(int v = 0) : value(v) {}
    
    int Add(int x) const { return value + x; }
    void SetValue(int v) { value = v; }
    int ThrowException() const { throw std::runtime_error("成员函数异常"); }
};

// 可调用对象（函数对象）
struct Functor {
    int operator()(int x) const {
        return x + 10;
    }
};

// 测试用的错误回调计数器
static int g_error_callback_count = 0;
static std::string g_last_error_context;
static std::string g_last_error_msg;

void CustomErrorCallback(const std::string& context, const std::string& error_msg) {
    g_error_callback_count++;
    g_last_error_context = context;
    g_last_error_msg = error_msg;
    fprintf(stderr, "[safe_call] [自定义错误回调] Context: %s, Error: %s\n", context.c_str(), error_msg.c_str());
}

void ResetErrorCallbackState() {
    g_error_callback_count = 0;
    g_last_error_context.clear();
    g_last_error_msg.clear();
}

// ========== 测试用例 ==========

// 测试正常函数调用：验证 SafeCall 能够正确调用带返回值的函数并返回正确的结果
CASE(TestNormalFunctionCall)
{
    fprintf(stderr, "[safe_call] \n=== 测试正常函数调用 ===\n");
    
    // 测试带返回值的函数
    auto result = ToolBox::SafeCall(NormalFunction, "test_context", nullptr, 5);
    assert(result.first == true);
    assert(result.second == 10);
    fprintf(stderr, "[safe_call] 正常函数调用成功: NormalFunction(5) = %d\n", result.second);
}

// 测试 void 函数调用：验证 SafeCall 能够正确处理无返回值的函数
CASE(TestVoidFunctionCall)
{
    fprintf(stderr, "[safe_call] \n=== 测试 void 函数调用 ===\n");
    
    int value = 0;
    auto result = ToolBox::SafeCall(NormalVoidFunction, "test_context", nullptr, std::ref(value));
    assert(result.first == true);
    assert(value == 42);
    fprintf(stderr, "[safe_call] void 函数调用成功: value = %d\n", value);
}

// 测试 std::exception 异常处理：验证 SafeCall 能够捕获标准异常并调用错误回调
CASE(TestStdExceptionHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试 std::exception 异常处理 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试带返回值的函数抛出异常
    auto result = ToolBox::SafeCall(ThrowStdException, "exception_test", CustomErrorCallback, 10);
    assert(result.first == false);
    assert(g_error_callback_count == 1);
    assert(g_last_error_context == "exception_test");
    assert(g_last_error_msg.find("std::exception") != std::string::npos);
    fprintf(stderr, "[safe_call] std::exception 捕获成功\n");
}

// 测试未知异常处理：验证 SafeCall 能够捕获非标准异常类型
CASE(TestUnknownExceptionHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试未知异常处理 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试未知异常
    auto result = ToolBox::SafeCall(ThrowUnknownException, "unknown_exception_test", CustomErrorCallback, 10);
    assert(result.first == false);
    assert(g_error_callback_count == 1);
    assert(g_last_error_msg == "unknown exception");
    fprintf(stderr, "[safe_call] 未知异常捕获成功\n");
}

// 测试 void 函数异常处理：验证无返回值函数抛出异常时的处理
CASE(TestVoidExceptionHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试 void 函数异常处理 ===\n");
    
    ResetErrorCallbackState();
    
    auto result = ToolBox::SafeCall(VoidThrowException, "void_exception_test", CustomErrorCallback);
    assert(result.first == false);
    assert(g_error_callback_count == 1);
    fprintf(stderr, "[safe_call] void 函数异常捕获成功\n");
}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
// 测试 SIGSEGV 信号处理：验证 SafeCall 能够捕获段错误信号（仅 Linux/macOS）
CASE(TestSIGSEGVHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试 SIGSEGV 信号处理 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试 SIGSEGV 捕获
    auto result = ToolBox::SafeCall(TriggerSIGSEGV, "sigsegv_test", CustomErrorCallback);
    assert(result.first == false);
    
    // 验证错误回调被调用且只调用一次
    assert(g_error_callback_count == 1);
    assert(g_last_error_msg == "caught signal (SIGSEGV/SIGFPE)");
    fprintf(stderr, "[safe_call] SIGSEGV 信号捕获成功\n");
}

// 测试 SIGFPE 信号处理：验证 SafeCall 能够捕获浮点异常信号（仅 Linux/macOS）
CASE(TestSIGFPEHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试 SIGFPE 信号处理 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试 SIGFPE 捕获
    auto result = ToolBox::SafeCall(TriggerSIGFPE, "sigfpe_test", CustomErrorCallback);
    assert(result.first == false);
    
    // 验证错误回调被调用且只调用一次
    assert(g_error_callback_count == 1);
    assert(g_last_error_msg == "caught signal (SIGSEGV/SIGFPE)");
    fprintf(stderr, "[safe_call] SIGFPE 信号捕获成功\n");
}

// 测试 void 函数 SIGSEGV 处理：验证无返回值函数触发段错误时的处理（仅 Linux/macOS）
CASE(TestVoidSIGSEGVHandling)
{
    fprintf(stderr, "[safe_call] \n=== 测试 void 函数 SIGSEGV 处理 ===\n");
    
    ResetErrorCallbackState();
    
    auto result = ToolBox::SafeCall(VoidTriggerSIGSEGV, "void_sigsegv_test", CustomErrorCallback);
    assert(result.first == false);
    
    // 验证错误回调被调用且只调用一次
    assert(g_error_callback_count == 1);
    assert(g_last_error_msg == "caught signal (SIGSEGV/SIGFPE)");
    fprintf(stderr, "[safe_call] void 函数 SIGSEGV 捕获成功\n");
}
#endif

// 测试 Lambda 函数：验证 SafeCall 能够正确处理 lambda 表达式
CASE(TestLambdaFunction)
{
    fprintf(stderr, "[safe_call] \n=== 测试 Lambda 函数 ===\n");
    
    // 测试 lambda 表达式
    auto lambda = [](int x, int y) -> int {
        return x + y;
    };
    
    auto result = ToolBox::SafeCall(lambda, "lambda_test", nullptr, 3, 4);
    assert(result.first == true);
    assert(result.second == 7);
    fprintf(stderr, "[safe_call] Lambda 函数调用成功: result = %d\n", result.second);
}

// 测试 std::function：验证 SafeCall 能够正确处理 std::function 对象
CASE(TestStdFunction)
{
    fprintf(stderr, "[safe_call] \n=== 测试 std::function ===\n");
    
    std::function<int(int)> func = [](int x) { return x * 3; };
    
    auto result = ToolBox::SafeCall(func, "std_function_test", nullptr, 7);
    assert(result.first == true);
    assert(result.second == 21);
    fprintf(stderr, "[safe_call] std::function 调用成功: result = %d\n", result.second);
}

// 测试函数对象 (Functor)：验证 SafeCall 能够正确处理函数对象
CASE(TestFunctor)
{
    fprintf(stderr, "[safe_call] \n=== 测试函数对象 (Functor) ===\n");
    
    Functor functor;
    auto result = ToolBox::SafeCall(functor, "functor_test", nullptr, 15);
    assert(result.first == true);
    assert(result.second == 25);
    fprintf(stderr, "[safe_call] Functor 调用成功: result = %d\n", result.second);
}

// 测试成员函数：验证 SafeCall 能够通过 lambda 包装调用类成员函数
CASE(TestMemberFunction)
{
    fprintf(stderr, "[safe_call] \n=== 测试成员函数 ===\n");
    
    TestClass obj(20);
    
    // 测试 const 成员函数 - 使用 lambda 包装
    auto result1 = ToolBox::SafeCall([&obj](int x) { return obj.Add(x); }, "member_function_test", nullptr, 5);
    assert(result1.first == true);
    assert(result1.second == 25);
    fprintf(stderr, "[safe_call] 成员函数调用成功: obj.Add(5) = %d\n", result1.second);
    
    // 测试非 const 成员函数 - 使用 lambda 包装
    auto result2 = ToolBox::SafeCall([&obj](int v) { obj.SetValue(v); }, "member_function_test2", nullptr, 100);
    assert(result2.first == true);
    assert(obj.value == 100);
    fprintf(stderr, "[safe_call] 成员函数调用成功: obj.value = %d\n", obj.value);
}

// 测试成员函数异常：验证成员函数抛出异常时的处理
CASE(TestMemberFunctionException)
{
    fprintf(stderr, "[safe_call] \n=== 测试成员函数异常 ===\n");
    
    ResetErrorCallbackState();
    
    TestClass obj(10);
    // 使用 lambda 包装成员函数
    auto result = ToolBox::SafeCall([&obj]() { return obj.ThrowException(); }, "member_exception_test", CustomErrorCallback);
    assert(result.first == false);
    assert(g_error_callback_count == 1);
    fprintf(stderr, "[safe_call] 成员函数异常捕获成功\n");
}

// 测试多参数函数：验证 SafeCall 能够正确处理带多个参数的函数
CASE(TestMultipleParameters)
{
    fprintf(stderr, "[safe_call] \n=== 测试多参数函数 ===\n");
    
    auto multi_func = [](int a, int b, int c, int d) -> int {
        return a + b + c + d;
    };
    
    auto result = ToolBox::SafeCall(multi_func, "multi_param_test", nullptr, 1, 2, 3, 4);
    assert(result.first == true);
    assert(result.second == 10);
    fprintf(stderr, "[safe_call] 多参数函数调用成功: result = %d\n", result.second);
}

// 测试不同返回值类型：验证 SafeCall 能够正确处理各种返回类型（bool, string, vector 等）
CASE(TestReturnValueTypes)
{
    fprintf(stderr, "[safe_call] \n=== 测试不同返回值类型 ===\n");
    
    // 测试返回 bool
    auto bool_func = [](bool x) -> bool { return !x; };
    auto bool_result = ToolBox::SafeCall(bool_func, "bool_test", nullptr, true);
    assert(bool_result.first == true);
    assert(bool_result.second == false);
    
    // 测试返回 string
    auto string_func = [](const std::string& s) -> std::string { return s + "_suffix"; };
    auto string_result = ToolBox::SafeCall(string_func, "string_test", nullptr, std::string("test"));
    assert(string_result.first == true);
    assert(string_result.second == "test_suffix");
    
    // 测试返回 vector
    auto vector_func = [](int size) -> std::vector<int> {
        return std::vector<int>(size, 42);
    };
    auto vector_result = ToolBox::SafeCall(vector_func, "vector_test", nullptr, 5);
    assert(vector_result.first == true);
    assert(vector_result.second.size() == 5);
    
    fprintf(stderr, "[safe_call] 不同返回值类型测试成功\n");
}

// 测试 SafeCallSimple：验证简化版本 SafeCallSimple 的功能（只返回成功/失败）
CASE(TestSafeCallSimple)
{
    fprintf(stderr, "[safe_call] \n=== 测试 SafeCallSimple ===\n");
    
    // 测试成功调用
    bool success = ToolBox::SafeCallSimple(NormalFunction, "simple_test", nullptr, 10);
    assert(success == true);
    
    // 测试异常调用
    ResetErrorCallbackState();
    success = ToolBox::SafeCallSimple(ThrowStdException, "simple_exception_test", CustomErrorCallback, 10);
    assert(success == false);
    assert(g_error_callback_count == 1);
    
    fprintf(stderr, "[safe_call] SafeCallSimple 测试成功\n");
}

// 测试默认错误回调：验证 SafeCall 使用默认错误回调时的行为
CASE(TestDefaultErrorCallback)
{
    fprintf(stderr, "[safe_call] \n=== 测试默认错误回调 ===\n");
    
    // 使用默认错误回调（不传入自定义回调）
    auto result = ToolBox::SafeCall(ThrowStdException, "default_callback_test", nullptr, 10);
    assert(result.first == false);
    
    // 测试默认回调参数
    auto result2 = ToolBox::SafeCall(ThrowStdException, "default_callback_test2", ToolBox::DefaultErrorCallback, 10);
    assert(result2.first == false);
    
    fprintf(stderr, "[safe_call] 默认错误回调测试成功\n");
}

// 测试空错误回调：验证传入 nullptr 作为错误回调时不会崩溃
CASE(TestNullErrorCallback)
{
    fprintf(stderr, "[safe_call] \n=== 测试空错误回调 ===\n");
    
    // 传入 nullptr 作为错误回调，应该不会崩溃
    ToolBox::ErrorCallback null_callback = nullptr;
    auto result = ToolBox::SafeCall(ThrowStdException, "null_callback_test", null_callback, 10);
    assert(result.first == false);
    
    fprintf(stderr, "[safe_call] 空错误回调测试成功（无崩溃）\n");
}

// 测试复杂场景：验证 SafeCall 的嵌套调用等复杂使用场景
CASE(TestComplexScenario)
{
    fprintf(stderr, "[safe_call] \n=== 测试复杂场景 ===\n");
    
    // 测试嵌套调用
    auto outer_func = [](int x) -> int {
        auto result = ToolBox::SafeCall([](int y) { return y * 2; }, "inner_call", nullptr, x);
        if (result.first) {
            return result.second + 1;
        }
        return -1;
    };
    
    auto result = ToolBox::SafeCall(outer_func, "outer_call", nullptr, 5);
    assert(result.first == true);
    assert(result.second == 11);  // (5 * 2) + 1
    
    fprintf(stderr, "[safe_call] 复杂场景测试成功: result = %d\n", result.second);
}

// 测试异常消息内容：验证错误回调接收到的异常消息内容是否正确
CASE(TestExceptionMessageContent)
{
    fprintf(stderr, "[safe_call] \n=== 测试异常消息内容 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试异常消息是否包含异常信息
    ToolBox::SafeCall(ThrowStdException, "msg_test", CustomErrorCallback, 10);
    assert(g_last_error_msg.find("运行时错误") != std::string::npos || 
           g_last_error_msg.find("std::exception") != std::string::npos);
    
    fprintf(stderr, "[safe_call] 异常消息: %s\n", g_last_error_msg.c_str());
}

// 测试异常后的返回值：验证函数抛出异常后返回的默认构造值是否正确
CASE(TestReturnValueAfterException)
{
    fprintf(stderr, "[safe_call] \n=== 测试异常后的返回值 ===\n");
    
    // 对于抛出异常的函数，返回值应该是默认构造的
    auto result = ToolBox::SafeCall(ThrowStdException, "return_test", nullptr, 10);
    assert(result.first == false);
    assert(result.second == 0);  // int 的默认值
    
    // 对于 string 类型
    auto string_throw = [](const std::string&) -> std::string {
        throw std::runtime_error("test");
    };
    auto string_result = ToolBox::SafeCall(string_throw, "string_return_test", nullptr, std::string("test"));
    assert(string_result.first == false);
    assert(string_result.second == "");  // string 的默认值（空字符串）
    
    fprintf(stderr, "[safe_call] 异常后返回值测试成功\n");
}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
// 测试信号和异常的捕获顺序：验证信号和异常处理的优先级（仅 Linux/macOS）
CASE(TestSignalAndExceptionOrder)
{
    fprintf(stderr, "[safe_call] \n=== 测试信号和异常的捕获顺序 ===\n");
    
    ResetErrorCallbackState();
    
    // 测试信号处理是否正常工作
    // 注意：实际测试中，如果函数抛出异常，不会触发信号
    // 如果函数触发信号，longjmp 会跳过异常处理
    auto result = ToolBox::SafeCall(TriggerSIGSEGV, "signal_order_test", CustomErrorCallback);
    assert(result.first == false);
    
    fprintf(stderr, "[safe_call] 信号捕获测试完成\n");
}
#endif

// 测试函数指针：验证 SafeCall 能够正确处理函数指针
CASE(TestFunctionPointer)
{
    fprintf(stderr, "[safe_call] \n=== 测试函数指针 ===\n");
    
    int (*func_ptr)(int) = NormalFunction;
    auto result = ToolBox::SafeCall(func_ptr, "func_ptr_test", nullptr, 8);
    assert(result.first == true);
    assert(result.second == 16);
    
    fprintf(stderr, "[safe_call] 函数指针测试成功: result = %d\n", result.second);
}

// 测试引用参数：验证 SafeCall 能够正确处理引用类型的参数
CASE(TestReferenceParameters)
{
    fprintf(stderr, "[safe_call] \n=== 测试引用参数 ===\n");
    
    int value = 0;
    auto modify_func = [](int& val) {
        val = 100;
    };
    
    auto result = ToolBox::SafeCall(modify_func, "ref_param_test", nullptr, std::ref(value));
    assert(result.first == true);
    assert(value == 100);
    
    fprintf(stderr, "[safe_call] 引用参数测试成功: value = %d\n", value);
}

// 测试常量引用参数：验证 SafeCall 能够正确处理常量引用类型的参数
CASE(TestConstReferenceParameters)
{
    fprintf(stderr, "[safe_call] \n=== 测试常量引用参数 ===\n");
    
    const std::string str = "test";
    auto read_func = [](const std::string& s) -> size_t {
        return s.length();
    };
    
    auto result = ToolBox::SafeCall(read_func, "const_ref_test", nullptr, std::cref(str));
    assert(result.first == true);
    assert(result.second == 4);
    
    fprintf(stderr, "[safe_call] 常量引用参数测试成功\n");
}

FIXTURE_END(TestSafeCall)

