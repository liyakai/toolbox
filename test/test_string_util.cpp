#include "src/tools/smart_pointer.h"
#include "unit_test_frame/unittest.h"


FIXTURE_BEGIN(TestSmartPointer)

// 使用默认空格分隔符
CASE(test_space_delimiters)
{
    std::vector<std::string> result;
    std::string string_with_space = " li ya kai too box .";
    Split(string_with_space, result);
    fprintf(stderr, "[字符串分割测试] 默认空格分割测试结果:\n");
    for (auto &str : result)
    {
        fprintf(stderr, "%s\n", str.c_str());
    }
    
}




FIXTURE_END(TestSmartPointer)