#include "tools/function_traits.h"
#include "unit_test_frame/unittest.h"


FIXTURE_BEGIN(FunctionTraits)


// Test functions
int test_func(int a, double b) { return 0; }
std::string test_func2() { return ""; }

// Test class
class TestClass {
public:
    void member_func(int a, float b) {}
    int const_member_func(std::string s) const { return 0; }
};

CASE(FunctionTraitsTest_RegularFunction) {
    // Test regular function with multiple parameters
    using traits1 = ToolBox::FunctionTraits<decltype(test_func)>;
    static_assert(std::is_same_v<traits1::return_type, int>);
    static_assert(traits1::arity == 2);
    static_assert(std::is_same_v<traits1::args_tuple, std::tuple<int, double>>);

    // Test regular function with no parameters
    using traits2 = ToolBox::FunctionTraits<decltype(test_func2)>;
    static_assert(std::is_same_v<traits2::return_type, std::string>);
    static_assert(traits2::arity == 0);
    static_assert(std::is_same_v<traits2::args_tuple, std::tuple<>>);
}

CASE(FunctionTraitsTest_MemberFunction) {
    // Test non-const member function
    using traits1 = ToolBox::FunctionTraits<decltype(&TestClass::member_func)>;
    static_assert(std::is_same_v<traits1::return_type, void>);
    static_assert(traits1::arity == 2);
    static_assert(std::is_same_v<traits1::args_tuple, std::tuple<int, float>>);

    // Test const member function
    using traits2 = ToolBox::FunctionTraits<decltype(&TestClass::const_member_func)>;
    static_assert(std::is_same_v<traits2::return_type, int>);
    static_assert(traits2::arity == 1);
    static_assert(std::is_same_v<traits2::args_tuple, std::tuple<std::string>>);
}

CASE(FunctionTraitsTest_Lambda) {
    auto lambda1 = [](int x, double y) -> float { return 0.0f; };
    using traits1 = ToolBox::FunctionTraits<decltype(lambda1)>;
    static_assert(std::is_same_v<traits1::return_type, float>);
    static_assert(traits1::arity == 2);
    static_assert(std::is_same_v<traits1::args_tuple, std::tuple<int, double>>);

    auto lambda2 = []() -> void {};
    using traits2 = ToolBox::FunctionTraits<decltype(lambda2)>;
    static_assert(std::is_same_v<traits2::return_type, void>);
    static_assert(traits2::arity == 0);
    static_assert(std::is_same_v<traits2::args_tuple, std::tuple<>>);
}

FIXTURE_END(FunctionTraits)