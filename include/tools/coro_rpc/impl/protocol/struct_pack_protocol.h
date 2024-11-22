#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>
namespace ToolBox::CoroRpc
{

class StructPackTools
{
public:
    template<typename ResultType = std::vector<std::byte>, typename... Args>
    [[nodiscard]]
    static ResultType Serialize(const Args&... args)
    {
        ResultType result;

        return result;
    }

    template<typename T, typename... Args, typename View>
    [[nodiscard]]
    static  bool Deserialize(T&t, View v, Args&... args)
    {
        return true;
    }
};

class StructPackProtocol
{
public:
    template<typename T>
    static std::string Serialize(const T &t)
    {
        return StructPackTools::Serialize<std::string>(t);
    }
    static std::string Serialize()
    {
        return StructPackTools::Serialize<std::string>(std::monostate{});
    }
    template<typename T>
    static bool Deserialize(T&t, std::string_view buffer)
    {
        if constexpr(std::tuple_size_v<T> == 1)
        {
            return StructPackTools::Deserialize(std::get<0>(t), buffer);
        }else {
            return StructPackTools::Deserialize(t, buffer);
        }
    }
};
}   // namespace ToolBox::CoroRpc