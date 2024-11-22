#pragma once

#include <tuple>
#include <cstddef>

namespace ToolBox {


// Add FunctionTraits definition
template<typename F>
struct FunctionTraits;

template <typename Return, typename... Arguments>
struct FunctionTraits<Return (*)(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct FunctionTraits<Return (*)(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct FunctionTraits<Return(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct FunctionTraits<Return(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename This, typename Return, typename... Arguments>
struct FunctionTraits<Return (This::*)(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct FunctionTraits<Return (This::*)(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct FunctionTraits<Return (This::*)(Arguments...) const> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct FunctionTraits<Return (This::*)(Arguments...) const noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename Return>
struct FunctionTraits<Return (*)()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct FunctionTraits<Return (*)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct FunctionTraits<Return (&)()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct FunctionTraits<Return (&)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct FunctionTraits<Return()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct FunctionTraits<Return() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename This, typename Return>
struct FunctionTraits<Return (This::*)()> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct FunctionTraits<Return (This::*)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct FunctionTraits<Return (This::*)() const> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct FunctionTraits<Return (This::*)() const noexcept> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

// Support function object and lambda expression
template <class Function>
struct FunctionTraits : FunctionTraits<decltype(&Function::operator())> {};

template <typename Function>
using function_parameters_t =
    typename FunctionTraits<std::remove_cvref_t<Function>>::parameters_type;


template <typename Function>
using last_parameters_type_t =
    std::tuple_element_t<std::tuple_size_v<function_parameters_t<Function>> - 1,
                         function_parameters_t<Function>>;

template <typename Function>
using function_return_type_t =
    typename FunctionTraits<std::remove_cvref_t<Function>>::return_type;

template <typename Function>
using class_type_t =
    typename FunctionTraits<std::remove_cvref_t<Function>>::class_type;

template <typename F, typename... Args>
struct is_invocable
    : std::is_constructible<
          std::function<void(std::remove_reference_t<Args>...)>,
          std::reference_wrapper<typename std::remove_reference<F>::type>> {};

template <typename F, typename... Args>
inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

template <typename T>
struct remove_first {
  using type = T;
};

template <class First, class... Second>
struct remove_first<std::tuple<First, Second...>> {
  using type = std::tuple<Second...>;
};

template <typename T>
using remove_first_t = typename remove_first<T>::type;

template <bool has_conn, typename T>
inline auto get_args() {
  if constexpr (has_conn) {
    using args_type = remove_first_t<T>;
    return args_type{};
  }
  else {
    return T{};
  }
}

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template <typename Test, template <typename...> class Ref>
inline constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

}   // namespace ToolBox