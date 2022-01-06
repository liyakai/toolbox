#include "src/tools/virtual_print.h"
#include "unit_test_frame/unittest.h"


FIXTURE_BEGIN(VirtualPrint)

// 打印 int 类型的虚函数.[结果为没有虚函数]
CASE(PrintInt)
{
    int obj = 0;
    Virtual<int> v_obj(obj);
    v_obj.SetDebugPrint(false);
    v_obj.PrintVirtFunc(); 
}
// 定义 基类
class Base
{
public:
    // virtual ~Base(){};
    virtual void func1(){ printf("Base::func1\n"); };
    virtual void func2(){ printf("Base::func2\n"); };
};
// 定义 派生类
class Derive : public Base
{
public:
    virtual void func1(){ printf("Derive::func1\n"); };
    virtual void func3(){ printf("Derive::func3\n"); };
    virtual void func4(){ printf("Derive::func4\n"); };
};
// 打印 Base 类型的虚函数.
CASE(PrintBase)
{
    Base obj;
    Virtual<Base> v_obj(obj);
    v_obj.SetDebugPrint(true);
    v_obj.PrintVirtFunc();
}
// 打印 Derive 类型的虚函数.
CASE(PrintDerive)
{
    Derive obj;
    Virtual<Derive> v_obj(obj);
    v_obj.SetDebugPrint(true);
    v_obj.PrintVirtFunc(); 
    // std::this_thread::sleep_for(std::chrono::milliseconds(10000000));
}


FIXTURE_END(VirtualPrint)