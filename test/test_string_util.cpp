#include "tools/string_util.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestStringUtil)

// 使用默认空格分隔符
CASE(test_space_delimiters)
{
    std::vector<std::string> result;
    std::string string_with_space = " li ya kai tool box .";
    ToolBox::Split(string_with_space, result);
    fprintf(stderr, "[字符串分割测试] 默认空格分割测试原始字符串[%s],分割结果:\n", string_with_space.c_str());
    for (auto& str : result)
    {
        fprintf(stderr, "%s\n", str.c_str());
    }

}

// 使用多分隔符"|"
CASE(test_char_delimiters)
{
    std::vector<std::string> result;
    std::string string_with_space = " li|ya| kai |tool||| box|||||| .";
    ToolBox::Split(string_with_space, result, "||");
    fprintf(stderr, "[字符串分割测试] 竖线\"|\"分隔符测试原始字符串[%s],分割结果:\n", string_with_space.c_str());
    for (auto& str : result)
    {
        fprintf(stderr, "%s\n", str.c_str());
    }
}



FIXTURE_END(TestStringUtil)