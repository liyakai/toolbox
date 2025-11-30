#pragma once
#include <string_view>
#include <cstdio>

// 获取函数名

namespace ToolBox
{

namespace detail {
    // 内部辅助函数：从编译器内部函数名中提取原始函数名（带命名空间）
    template<auto func>
    constexpr const std::string_view ExtractFuncNameFromSignature()
    {
#if defined(_MSC_VER) // MSVC
        constexpr std::string_view full_name = __FUNCSIG__;
        // Find last space before function name and closing '>'
        constexpr size_t start = full_name.rfind(' ', full_name.find('>')) + 1;
        constexpr size_t end = full_name.find('<', start);
        return full_name.substr(start, end - start);
#elif defined(__clang__) || defined(__GNUC__) // Clang or GCC 
        constexpr std::string_view full_name = __PRETTY_FUNCTION__;
        // Find the function name between "GetFuncName() [with auto func = " and "]"
        constexpr size_t start = full_name.find('[') + 1;
        constexpr size_t end = full_name.find(']');
        constexpr size_t func_start = full_name.find('=', start) + 2;
        return full_name.substr(func_start, end - func_start);
#else
        #error "Unsupported compiler"
#endif
    }

    // 辅助函数：提取最后一个 '::' 之后的部分（纯函数名）
    template<auto func>
    constexpr const std::string_view ExtractLastName()
    {
        constexpr std::string_view full_name = ExtractFuncNameFromSignature<func>();
        constexpr size_t last_scope = full_name.rfind("::");
        return last_scope == std::string_view::npos ? 
            full_name : full_name.substr(last_scope + 2);
    }

    // 辅助函数：提取类名和函数名（跳过命名空间）
    template<auto func>
    constexpr const std::string_view ExtractClassAndMethod()
    {
        constexpr std::string_view full_name = ExtractFuncNameFromSignature<func>();
        constexpr size_t last_scope = full_name.rfind("::");
        // If no '::' found, return as is (plain function)
        if constexpr (last_scope == std::string_view::npos) {
            return full_name;
        }
        // Check if there's another '::' before the last one (indicating namespace::class::method)
        constexpr std::string_view before_last = full_name.substr(0, last_scope);
        constexpr size_t second_last_scope = before_last.rfind("::");
        // If only one '::', return from the beginning (class::method)
        if constexpr (second_last_scope == std::string_view::npos) {
            return full_name;
        }
        // Multiple '::', return from the second last (class::method, skipping namespace)
        return full_name.substr(second_last_scope + 2);
    }
}

// 获取带命名空间的完整函数名
template<auto func>
constexpr const std::string_view GetFuncNameWithNamespace()
{
    return detail::ExtractFuncNameFromSignature<func>();
}

// 获取纯函数名（去掉命名空间和类名）
template<auto func>
constexpr const std::string_view GetFuncName()
{
    return detail::ExtractLastName<func>();
}

// 获取带类名的函数名（如 ClassName::methodName，跳过命名空间）
template<auto func>
constexpr const std::string_view GetFuncNameWithClass()
{
    return detail::ExtractClassAndMethod<func>();
}

} // namespace ToolBox