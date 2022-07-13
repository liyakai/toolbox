#include "src/tools/smart_pointer.h"
#include "unit_test_frame/unittest.h"


FIXTURE_BEGIN(TestSmartPointer)

CASE(test_normal)
{
    // 定义一个基础类对象指针
    int32_t *obj = new int32_t(3);

    //定义三个智能指针类对象，对象都指向基础类对象 obj
    //使用花括号控制三个指针指针的ß生命期，观察计数的变化
    {
        ToolBox::SmartPtr<int32_t> sptr1(obj);    // 调用普通构造函数,此时计数器 strong_count 为 1
        {
            ToolBox::SmartPtr<int32_t> sptr2(sptr1);   // 调用拷贝构造函数,此时计数器 strong_counter 为 2
            {
                ToolBox::SmartPtr<int32_t> sptr3 = sptr1;     // 调用赋值操作符,此时计数器 strong_counter 为 3
            }
        }
    }
    fprintf(stderr, "[智能指针测试] obj_ptr%p obj_value:%d\n", obj, *obj);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

FIXTURE_END(TestSmartPointer)