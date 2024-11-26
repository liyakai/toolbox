#pragma once

#include "tools/log.h"
#include <cstdint>
#include <string>
#include <string_view>
#include "tools/function_name.h"
#include "tools/md5.h"

#define RpcLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
#define RpcLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
#define RpcLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
#define RpcLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
#define RpcLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
#define RpcLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)


namespace ToolBox::CoroRpc
{
    enum class Errc: uint16_t
    {
        SUCCESS = 0,
        ERR_TIMEOUT = 1,
        ERR_INVALID_ARGUMENTS = 2,
        ERR_FUNC_NOT_REGISTERED = 3,
        ERR_PROTOCOL = 4,
        ERR_OPERATION_CANCELED = 5,
        ERR_SERIAL_NUMBER_CONFLICT = 6,
        ERR_MESSAGE_TOO_LARGE = 7,
        ERR_HANDLER_THROW_EXCEPTION = 8,
        ERR_SEND_CALLBACK_NOT_SET = 9,
        ERR_SERVER_PREPARE_HEADER_FAILED = 10,
    };

    
class CoroRpcTools
{
public:
    using rpc_func_key_t = uint32_t;
    using rpc_func_key = rpc_func_key_t;
    // 基础类型的序列化
    template<typename T>
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, T &&arg)
    {
        const T& value = arg;
        const std::byte*  p_bytes = reinterpret_cast<const std::byte*>(&value);
        std::copy(p_bytes, p_bytes + sizeof(T), buffer.begin() + offset);
        offset += sizeof(T);
    }

    // 字符串类型的特化版本
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, std::string str)
    {
        // 先序列化字符串长度
        uint32_t length = str.size();
        SerializeArg(buffer, offset, length);

        // 确保 buffer 有足够的空间
        if(offset + length > buffer.size())
        {
            buffer.resize(offset + str.size());
        }
        // 再序列化字符串内容
        const std::byte* bytes = reinterpret_cast<const std::byte*>(str.data());
        std::copy(bytes, bytes + length, buffer.begin() + offset);
        offset += str.size();
    }

    // 基础类型的反序列化
    template<typename T>
    static T DeserializeArg(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        T value;
        std::byte* p_bytes = reinterpret_cast<std::byte*>(&value);
        std::copy(buffer.begin() + offset, buffer.begin() + offset + sizeof(T), p_bytes);
        offset += sizeof(T);
        return value;
    }

    // 字符串类型的反序列化特化版本
    static std::string DeserializeString(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        // 先反序列化字符串长度
        uint32_t length = DeserializeArg<uint32_t>(buffer, offset);

        // 反序列化字符串内容
        std::string result(reinterpret_cast<const char*>(buffer.data() + offset), length);
        offset += length;
        return result;
    }

        template<auto func>
    static rpc_func_key AutoGenRegisterKey()
    {
        constexpr auto name = ToolBox::GetFuncName<func>();
        auto id = MD5Hash32Constexpr(name);
        return static_cast<rpc_func_key>(id);
    }
};

}   // namespace ToolBox::CoroRpc
