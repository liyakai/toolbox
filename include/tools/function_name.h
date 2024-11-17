#pragma once
#include <string_view>
#include <cstdio>

// 获取函数名

namespace ToolBox
{

template<auto func>
constexpr const std::string_view GetFuncName()
{
#if defined(_MSC_VER) // MSVC
    constexpr std::string_view full_name = __FUNCSIG__;
    // Find last space before function name and closing '>'
    constexpr size_t start = full_name.rfind(' ', full_name.find('>')) + 1;
    constexpr size_t end = full_name.find('<', start);
    return full_name.substr(start, end - start);
#elif defined(__clang__) || defined(__GNUC__) // Clang or GCC 
    constexpr std::string_view full_name = __PRETTY_FUNCTION__;
    fprintf(stderr, "full_name: %s\n", full_name.data());
    // Find the function name between "GetFuncName() [with auto func = " and "]"
    constexpr size_t start = full_name.find('[') + 1;
    constexpr size_t end = full_name.find(']');
    constexpr size_t func_start = full_name.find('=', start) + 2;
    return full_name.substr(func_start, end - func_start);
#else
    #error "Unsupported compiler"
#endif
}

} // namespace ToolBox