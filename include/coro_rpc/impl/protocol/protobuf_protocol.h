#pragma once

#include <tuple>
#include <string_view>
#include <type_traits>
#include "../coro_rpc_def_interenal.h"
#include "tools/log.h"
#include <google/protobuf/message.h>

namespace ToolBox::CoroRpc
{

class ProtobufProtocol
{
public:
    template<typename T>
    requires std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<T>>
    static std::string Serialize(const T &t)
    {
        return t.SerializeAsString();
    }
    static std::string Serialize()
    {
        return "";
    }
    template<typename T>
    requires std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<T>>
    static Errc SerializeToBuffer(void* data, int size, const T &t)
    {
        if(t.SerializeToArray(data, size))
        {
            return CoroRpc::Errc::SUCCESS;
        }else {
            RpcLogError("[rpc][client] SerializeToBuffer failed, buffer size: %zu", size);
            return CoroRpc::Errc::ERR_SERIALIZE_FAILED;
        }
    }
    template<typename T>
    requires std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<T>>
    static size_t SerializeSize(const T &t)
    {
        return t.ByteSizeLong();
    }
    
    // 可变参数版本：将参数打包成tuple来序列化
    // 注意：Protobuf协议只支持protobuf message类型，不支持基本类型
    // 使用 SFINAE 确保只有 protobuf message 类型才能调用
    template<typename... Args>
    requires (sizeof...(Args) == 0 || (std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<Args>> && ...))
    static size_t SerializeSize(const Args&... args)
    {
        if constexpr(sizeof...(Args) == 0) {
            return 0;
        } else {
            return (args.ByteSizeLong() + ...);
        }
    }
    
    template<typename... Args>
    requires (sizeof...(Args) == 0 || (sizeof...(Args) == 1 && std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<std::tuple_element_t<0, std::tuple<Args...>>>>))
    static Errc SerializeToBuffer(void* data, int size, const Args&... args)
    {
        if constexpr(sizeof...(Args) == 0) {
            return CoroRpc::Errc::SUCCESS;
        } else {
            // 修复：使用折叠表达式获取第一个（也是唯一的）参数
            return SerializeToBuffer(data, size, std::get<0>(std::forward_as_tuple(args...)));
        }
    }
    
    template<typename T>
    static bool Deserialize(T&t, std::string_view buffer)
    {
        if constexpr(is_tuple_like_v<T>)
        {
            if constexpr(std::tuple_size_v<T> == 1)
            {
                using first_type = std::tuple_element_t<0, T>;
                if constexpr(std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<first_type>>) {
                    return std::get<0>(t).ParseFromArray(buffer.data(), buffer.size());
                } else {
                    // 基本类型不支持，应该使用StructPack协议
                    return false;
                }
            }else {
                // 多个元素的tuple，Protobuf不支持
                return false;
            }
        } else if constexpr(std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<T>>) {
            return t.ParseFromArray(buffer.data(), buffer.size());
        } else {
            // 对于基本类型，Protobuf不支持，应该使用StructPack协议
            // 这里返回false表示不支持
            return false;
        }
    }
};

} // namespace ToolBox::CoroRpc
