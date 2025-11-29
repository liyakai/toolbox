#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <cstring>
#include <type_traits>
#include <tuple>
#include "../coro_rpc_def_interenal.h"
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
        // 计算所需总大小
        size_t total_size = (sizeof(Args) + ...);
        result.reserve(total_size);
        
        // 使用 lambda 来序列化
        auto serialize_one = [&result](const auto& arg) {
            if constexpr (std::is_same_v<ResultType, std::string>) {
                // 对于 std::string，需要将 bytes 转换为 char
                const char* chars = reinterpret_cast<const char*>(&arg);
                result.append(chars, sizeof(arg));
            } else {
                const std::byte* bytes = reinterpret_cast<const std::byte*>(&arg);
                result.insert(result.end(), bytes, bytes + sizeof(arg));
            }
        };
        // 使用折叠表达式调用 lambda
        (serialize_one(args), ...);
        return result;
    }

    template<typename... Args>
    static size_t SerializeSize(const Args&... args)
    {
        return (sizeof(Args) + ...);
    }

    template<typename T, typename... Args, typename View>
    [[nodiscard]]
    static bool Deserialize(T&t, View v, Args&... args)
    {
        // 检查缓冲区大小是否足够 添加 0 作为初始值
        size_t required_size = sizeof(T) + (0 + ... + sizeof(Args));
        if (v.size() < required_size) {
            return false;
        }

        size_t offset = 0;

        // 反序列化第一个参数 T
        if constexpr (std::is_same_v<T, std::string_view>) {
            t = std::string_view(reinterpret_cast<const char*>(v.data()), v.size());
            offset += v.size();
        } else if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(&t, v.data(), sizeof(T));
            offset += sizeof(T);
        } else {
            return false;  // 不支持的类型
        }

        // 使用折叠表达式反序列化剩余参数
        bool success = true;
        auto deserialize_one = [&v, &offset, &success](auto& arg) {
            if constexpr (std::is_trivially_copyable_v<std::remove_reference_t<decltype(arg)>>) {
                if (offset + sizeof(arg) <= v.size()) {
                    std::memcpy(&arg, v.data() + offset, sizeof(arg));
                    offset += sizeof(arg);
                } else {
                    success = false;
                }
            } else {
                success = false;  // 不支持的类型
            }
        };

        (deserialize_one(args), ...);
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
    static Errc SerializeToBuffer(void* data, int size, const T &t)
    {
        std::string buffer = StructPackTools::Serialize<std::string>(t);
        if(buffer.size() > size)
        {
            return CoroRpc::Errc::ERR_BUFFER_TOO_SMALL;
        }
        std::memcpy(data, buffer.data(), buffer.size());
        return CoroRpc::Errc::SUCCESS;
    }
    
    // 可变参数版本：支持多个参数
    template<typename... Args>
    static Errc SerializeToBuffer(void* data, int size, const Args&... args)
    {
        std::string buffer = StructPackTools::Serialize<std::string>(args...);
        if(buffer.size() > size)
        {
            return CoroRpc::Errc::ERR_BUFFER_TOO_SMALL;
        }
        std::memcpy(data, buffer.data(), buffer.size());
        return CoroRpc::Errc::SUCCESS;
    }
    template<typename T>
    static size_t SerializeSize(const T &t)
    {
        return StructPackTools::SerializeSize(t);
    }
    template<typename T>
    static bool Deserialize(T&t, std::string_view buffer)
    {
        if constexpr( is_tuple_like_v<T>)
        {
            if constexpr(std::tuple_size_v<T> == 1)
            {
                return StructPackTools::Deserialize(std::get<0>(t), buffer);
            }else {
                return StructPackTools::Deserialize(t, buffer);
            }
        }else {
            return StructPackTools::Deserialize(t, buffer);
        }
    }
};
}   // namespace ToolBox::CoroRpc