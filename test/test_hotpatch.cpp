#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include "../include/tools/hotpatch.h"
#include "unit_test_frame/unittest.h"
#ifdef __linux__

FIXTURE_BEGIN(HotPatch)

// 测试函数
int simple_func(int x) { return x + 1; }
int test_function(int x) { return x * 2; }
int test_function2(int x) { 
    volatile int temp = x; 
    temp += 1; 
    temp *= 2; 
    return temp; 
}

// 测试类
class TestClass {
public:
    int value;
    
    TestClass(int v) : value(v) {}
    
    int getValue() const { return value; }
    int calculate(int x) const { return value + x; }
    int multiply(int x) const { return value * x; }
};

// 虚函数测试类
class VirtualTestClass {
public:
    int value;
    
    VirtualTestClass(int v) : value(v) {}
    virtual ~VirtualTestClass() = default;
    
    virtual int calculate(int x) const { return value + x; }
    virtual int multiply(int x) const { return value * x; }
    virtual void print() const { std::cout << "Value: " << value << std::endl; }
};

// 派生类
class DerivedVirtualClass : public VirtualTestClass {
public:
    DerivedVirtualClass(int v) : VirtualTestClass(v) {}
    
    virtual int calculate(int x) const override { return value * 2 + x; }
    virtual void print() const override { std::cout << "Derived Value: " << value << std::endl; }
};

// 补丁函数
extern "C" {
    int patched_simple_func(int x) { return x + 2; }
    int patched_test_function(int x) { return x * 3; }
    int safe_patched_test_function(int x) { return x * 3; }
    
    // 类成员函数的补丁函数
    int patched_calculate(const TestClass* obj, int x) { 
        return obj->value * 2 + x;  // 修改计算逻辑
    }
    int patched_multiply(const TestClass* obj, int x) { 
        return obj->value + x;  // 修改乘法为加法
    }
    
    // 虚函数的补丁函数
    int patched_virtual_calculate(const VirtualTestClass* obj, int x) { 
        return obj->value * 3 + x;  // 修改虚函数计算逻辑
    }
    int patched_virtual_multiply(const VirtualTestClass* obj, int x) { 
        return obj->value - x;  // 修改虚函数乘法逻辑
    }
    void patched_virtual_print(const VirtualTestClass* obj) { 
        std::cout << "PATCHED Value: " << obj->value << std::endl;  // 修改虚函数打印逻辑
    }
}

// 公共辅助函数
class TestHelper {
public:
    static bool create_and_compile_patch(const std::string& filename, const std::string& code) {
        FILE* fp = fopen(("/tmp/" + filename + ".cpp").c_str(), "w");
        if (!fp) return false;
        
        // 添加必要的头文件和类定义
        std::string full_code = generate_patch_header() + "\n" + code;
        fputs(full_code.c_str(), fp);
        fclose(fp);
        
        std::string cmd = "g++ -shared -fPIC -o /tmp/" + filename + ".so /tmp/" + filename + ".cpp";
        int result = system(cmd.c_str());
        return result == 0;
    }
    
    static std::string generate_patch_header() {
        return R"(
#include <iostream>

// 类定义（简化版本，只包含补丁函数需要的部分）
class TestClass {
public:
    int value;
};

class VirtualTestClass {
public:
    int value;
    virtual ~VirtualTestClass() = default;
    virtual int calculate(int x) const { return value + x; }
    virtual int multiply(int x) const { return value * x; }
    virtual void print() const { std::cout << "Value: " << value << std::endl; }
};

)";
    }
    
    static void cleanup(const std::string& filename) {
        unlink(("/tmp/" + filename + ".cpp").c_str());
        unlink(("/tmp/" + filename + ".so").c_str());
    }
    
    static void run_patch_test(const std::string& test_name, 
                              const std::string& filename,
                              const std::string& patch_code,
                              void* target_func,
                              const std::string& patch_func_name,
                              int test_value) {
        std::cout << "=== " << test_name << " ===" << std::endl;
        
        // 测试原始函数
        std::cout << "原始: " << test_value << " -> " << 
            (reinterpret_cast<int(*)(int)>(target_func))(test_value) << std::endl;
        
        // 创建并编译补丁
        if (!create_and_compile_patch(filename, patch_code)) {
            std::cerr << "编译失败" << std::endl;
            return;
        }
        
        // 执行热修补
        hotpatch::HotPatch hotpatch;
        if (!hotpatch.load_patch("/tmp/" + filename + ".so") ||
            !hotpatch.patch_function(patch_func_name, target_func)) {
            std::cerr << "热修补失败" << std::endl;
            cleanup(filename);
            return;
        }
        
        // 测试修补后的函数
        std::cout << "修补后: " << test_value << " -> " << 
            (reinterpret_cast<int(*)(int)>(target_func))(test_value) << std::endl;
        
        // 恢复并测试
        hotpatch.restore_function(target_func);
        std::cout << "恢复后: " << test_value << " -> " << 
            (reinterpret_cast<int(*)(int)>(target_func))(test_value) << std::endl;
        
        cleanup(filename);
        std::cout << "=== 完成 ===" << std::endl;
    }
};

CASE(TestMinimalPatch)
{
    const std::string patch_code = R"(
extern "C" {
    int patched_simple_func(int x) { return x + 2; }
}
)";
    
    TestHelper::run_patch_test("最小化热修补测试", "minimal_patch", patch_code, 
                              reinterpret_cast<void*>(simple_func), 
                              "patched_simple_func", 5);
}


CASE(TestSimplePatch)
{
    const std::string patch_code = R"(
extern "C" {
    int patched_test_function(int x) { return x * 3; }
}
)";
    
    TestHelper::run_patch_test("简单热修补测试", "simple_patch", patch_code, 
                              reinterpret_cast<void*>(test_function), 
                              "patched_test_function", 5);
}


CASE(TestSafePatch)
{
    const std::string patch_code = R"(
extern "C" {
    int safe_patched_test_function(int x) { return x * 3; }
}
)";
    
    TestHelper::run_patch_test("安全热修补测试", "safe_patch", patch_code, 
                              reinterpret_cast<void*>(test_function2), 
                              "safe_patched_test_function", 5);
}

CASE(TestMemberFunctionPatch)
{
    std::cout << "=== 类成员函数热修补测试 ===" << std::endl;
    
    // 创建测试对象
    TestClass obj(10);
    std::cout << "原始对象值: " << obj.value << std::endl;
    
    // 测试原始成员函数
    std::cout << "原始 calculate(5): " << obj.calculate(5) << std::endl;
    std::cout << "原始 multiply(3): " << obj.multiply(3) << std::endl;
    
    // 创建补丁代码
    const std::string patch_code = R"(
extern "C" {
    int patched_calculate(const TestClass* obj, int x) { 
        return obj->value * 2 + x;  // 修改计算逻辑
    }
    int patched_multiply(const TestClass* obj, int x) { 
        return obj->value + x;  // 修改乘法为加法
    }
}
)";
    
    // 编译补丁
    if (!TestHelper::create_and_compile_patch("member_patch", patch_code)) {
        std::cerr << "编译失败" << std::endl;
        return;
    }
    
    // 执行热修补
    hotpatch::HotPatch hotpatch;
    if (!hotpatch.load_patch("/tmp/member_patch.so")) {
        std::cerr << "加载补丁失败" << std::endl;
        TestHelper::cleanup("member_patch");
        return;
    }
    
    // 修补 calculate 方法
    // 注意：成员函数指针不能直接转换为void*，需要先转换为函数指针
    auto calculate_ptr = &TestClass::calculate;
    void* calculate_addr = *reinterpret_cast<void**>(&calculate_ptr);
    if (hotpatch.patch_function("patched_calculate", calculate_addr)) {
        std::cout << "calculate 方法修补成功" << std::endl;
        std::cout << "修补后 calculate(5): " << obj.calculate(5) << std::endl;
    } else {
        std::cerr << "calculate 方法修补失败" << std::endl;
    }
    
    // 修补 multiply 方法
    auto multiply_ptr = &TestClass::multiply;
    void* multiply_addr = *reinterpret_cast<void**>(&multiply_ptr);
    if (hotpatch.patch_function("patched_multiply", multiply_addr)) {
        std::cout << "multiply 方法修补成功" << std::endl;
        std::cout << "修补后 multiply(3): " << obj.multiply(3) << std::endl;
    } else {
        std::cerr << "multiply 方法修补失败" << std::endl;
    }
    
    // 恢复方法
    hotpatch.restore_function(calculate_addr);
    hotpatch.restore_function(multiply_addr);
    std::cout << "恢复后 calculate(5): " << obj.calculate(5) << std::endl;
    std::cout << "恢复后 multiply(3): " << obj.multiply(3) << std::endl;
    
    TestHelper::cleanup("member_patch");
    std::cout << "=== 类成员函数热修补测试完成 ===" << std::endl;
}

CASE(TestVirtualFunctionPatch)
{
    std::cout << "=== 虚函数热修补测试 ===" << std::endl;
    
    // 创建测试对象
    VirtualTestClass* obj = new VirtualTestClass(10);
    std::cout << "原始对象值: " << obj->value << std::endl;
    
    // 测试原始虚函数
    std::cout << "原始虚函数调用:" << std::endl;
    std::cout << "  calculate(5): " << obj->calculate(5) << std::endl;
    std::cout << "  multiply(3): " << obj->multiply(3) << std::endl;
    std::cout << "  print(): ";
    obj->print();
    
    // 创建补丁代码
    const std::string patch_code = R"(
extern "C" {
    int patched_virtual_calculate(const VirtualTestClass* obj, int x) { 
        return obj->value * 3 + x;  // 修改虚函数计算逻辑
    }
    int patched_virtual_multiply(const VirtualTestClass* obj, int x) { 
        return obj->value - x;  // 修改虚函数乘法逻辑
    }
    void patched_virtual_print(const VirtualTestClass* obj) { 
        std::cout << "PATCHED Value: " << obj->value << std::endl;  // 修改虚函数打印逻辑
    }
}
)";
    
    // 编译补丁
    if (!TestHelper::create_and_compile_patch("virtual_patch", patch_code)) {
        std::cerr << "编译失败" << std::endl;
        delete obj;
        return;
    }
    
    // 执行热修补
    hotpatch::HotPatch hotpatch;
    if (!hotpatch.load_patch("/tmp/virtual_patch.so")) {
        std::cerr << "加载补丁失败" << std::endl;
        TestHelper::cleanup("virtual_patch");
        delete obj;
        return;
    }
    
    // 修补虚函数
    // 注意：vtable索引需要根据实际的虚函数表布局确定
    // 这里假设calculate是第一个虚函数（索引0），multiply是第二个（索引1），print是第三个（索引2）
    if (hotpatch.patch_virtual_function("patched_virtual_calculate", obj, 0)) {
        std::cout << "calculate 虚函数修补成功" << std::endl;
    } else {
        std::cerr << "calculate 虚函数修补失败" << std::endl;
    }
    
    if (hotpatch.patch_virtual_function("patched_virtual_multiply", obj, 1)) {
        std::cout << "multiply 虚函数修补成功" << std::endl;
    } else {
        std::cerr << "multiply 虚函数修补失败" << std::endl;
    }
    
    if (hotpatch.patch_virtual_function("patched_virtual_print", obj, 2)) {
        std::cout << "print 虚函数修补成功" << std::endl;
    } else {
        std::cerr << "print 虚函数修补失败" << std::endl;
    }
    
    // 测试修补后的虚函数
    std::cout << "修补后虚函数调用:" << std::endl;
    std::cout << "  calculate(5): " << obj->calculate(5) << std::endl;
    std::cout << "  multiply(3): " << obj->multiply(3) << std::endl;
    std::cout << "  print(): ";
    obj->print();
    
    // 测试多态性
    std::cout << "测试多态性:" << std::endl;
    VirtualTestClass* derived = new DerivedVirtualClass(20);
    std::cout << "派生类对象:" << std::endl;
    std::cout << "  calculate(5): " << derived->calculate(5) << std::endl;
    std::cout << "  print(): ";
    derived->print();
    
    // 恢复虚函数
    hotpatch.restore_virtual_function(obj, 0);
    hotpatch.restore_virtual_function(obj, 1);
    hotpatch.restore_virtual_function(obj, 2);
    
    std::cout << "恢复后虚函数调用:" << std::endl;
    std::cout << "  calculate(5): " << obj->calculate(5) << std::endl;
    std::cout << "  multiply(3): " << obj->multiply(3) << std::endl;
    std::cout << "  print(): ";
    obj->print();
    
    TestHelper::cleanup("virtual_patch");
    delete obj;
    delete derived;
    std::cout << "=== 虚函数热修补测试完成 ===" << std::endl;

    sleep(1000000);
}



FIXTURE_END(HotPatch)

#endif // __linux__
