#pragma once

#include <tuple>
#include <string_view>
#include "../coro_rpc_def_interenal.h"
#include "tools/log.h"
// ::google::protobuf::Message

namespace ToolBox::CoroRpc
{

class ProtobufProtocol
{
public:
    template<typename T>
    static std::string Serialize(const T &t)
    {
        return t.SerializeAsString();
    }
    static std::string Serialize()
    {
        return "";
    }
    template<typename T>
    static Errc SerializeToBuffer(void* data, int size, const T &t)
    {
        fprintf(stderr, "protobuf SerializeToBuffer, buffer size: %d,  t:%s\n", size, t.DebugString().c_str());
        if(t.SerializeToArray(data, size))
        {
            return CoroRpc::Errc::SUCCESS;
        }else {
            RpcLogError("[rpc][client] SerializeToBuffer failed, buffer size: %zu", size);
            return CoroRpc::Errc::ERR_SERIALIZE_FAILED;
        }
    }
    template<typename T>
    static size_t SerializeSize(const T &t)
    {
        return t.ByteSizeLong();
    }
    template<typename T>
    static bool Deserialize(T&t, std::string_view buffer)
    {
        // fprintf(stderr, "protobuf Deserialize, buffer size: %zu, deserialize content:%s\n", buffer.size(), buffer.data());

        if constexpr(is_tuple_like_v<T>)
        {
            if constexpr(std::tuple_size_v<T> == 1)
            {
                std::get<0>(t).ParseFromArray(buffer.data(), buffer.size());
                // fprintf(stderr, "protobuf Deserialize, buffer size: %zu, t:%s\n", buffer.size(), std::get<0>(t).DebugString().c_str());
                return std::get<0>(t).ParseFromArray(buffer.data(), buffer.size());
            }else {
                t.ParseFromString(std::string(buffer));
                // fprintf(stderr, "protobuf Deserialize, buffer size: %zu, t:%s\n", buffer.size(), t.DebugString().c_str());
                return true;
            }
        }else {
            return t.ParseFromArray(buffer.data(), buffer.size());
        }
        
    }
};

} // namespace ToolBox::CoroRpc
