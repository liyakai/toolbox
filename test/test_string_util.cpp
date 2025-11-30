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

// VectorToStr 测试 - 默认逗号分隔符
CASE(test_vector_to_str_default_delimiter)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::string result = ToolBox::VectorToStr(vec);
    fprintf(stderr, "[Vector转字符串测试] 默认逗号分隔符测试 vector=[1,2,3,4,5], 结果=[%s]\n", result.c_str());
}

// VectorToStr 测试 - 自定义分隔符
CASE(test_vector_to_str_custom_delimiter)
{
    std::vector<int> vec = {10, 20, 30};
    std::string result = ToolBox::VectorToStr(vec, " | ");
    fprintf(stderr, "[Vector转字符串测试] 自定义分隔符测试 vector=[10,20,30], 分隔符=\" | \", 结果=[%s]\n", result.c_str());
}

// VectorToStr 测试 - double 类型
CASE(test_vector_to_str_double)
{
    std::vector<double> vec = {1.5, 2.7, 3.9};
    std::string result = ToolBox::VectorToStr(vec);
    fprintf(stderr, "[Vector转字符串测试] double类型测试 vector=[1.5,2.7,3.9], 结果=[%s]\n", result.c_str());
}

// VectorToStr 测试 - 空 vector
CASE(test_vector_to_str_empty)
{
    std::vector<int> vec;
    std::string result = ToolBox::VectorToStr(vec);
    fprintf(stderr, "[Vector转字符串测试] 空vector测试, 结果=[%s]\n", result.c_str());
}

// VectorToStr 测试 - 单个元素
CASE(test_vector_to_str_single_element)
{
    std::vector<int> vec = {42};
    std::string result = ToolBox::VectorToStr(vec);
    fprintf(stderr, "[Vector转字符串测试] 单个元素测试 vector=[42], 结果=[%s]\n", result.c_str());
}



FIXTURE_END(TestStringUtil)