#include "src/tools/debug_new.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestDebugNew)

/*
* 定义一个测试用类
*/
class TestClass
{
public:
private:
    int32_t m_a = 0;
    int32_t m_b = 1;
};

CASE(TestNewNormal){
    int* single = NEW int;
    CHECK_LEAKS
    delete single;
    CHECK_LEAKS

    int* array = NEW int[100];
    CHECK_LEAKS
    delete array;
    CHECK_LEAKS
    // 对于基本数据类型的回收,delete 和 delete[] 是一样的.  // blog: https://www.cnblogs.com/whwywzhj/p/7905176.html


    TestClass* single_class = NEW TestClass;
    CHECK_LEAKS
    delete single_class;
    CHECK_LEAKS

    TestClass* array_class = NEW TestClass[100];
    CHECK_LEAKS
    delete array_class;
    CHECK_LEAKS
    // 对于基本数据类型的回收,delete 和 delete[] 都能将内存正确回收,但delete只调用一个析构函数, delete[]会调用所有对象的析构函数  // blog: https://www.cnblogs.com/whwywzhj/p/7905176.html
}

FIXTURE_END(TestDebugNew)



