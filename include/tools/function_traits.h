#pragma once

#include <tuple>
#include <cstddef>

namespace ToolBox {


// Add function_traits definition
template<typename F>
struct function_traits;

// For regular functions
template<typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// For member functions
template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

}   // namespace ToolBox