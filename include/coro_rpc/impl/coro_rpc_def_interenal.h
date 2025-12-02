#pragma once

#include "tools/log.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <stdexcept>
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
        ERR_BUFFER_TOO_SMALL = 8,
        ERR_HANDLER_THROW_EXCEPTION = 9,
        ERR_SEND_CALLBACK_NOT_SET = 10,
        ERR_SERVER_PREPARE_HEADER_FAILED = 11,
        ERR_SERIALIZE_FAILED = 12,
        ERR_DESERIALIZE_FAILED = 13,
        ERR_PROTOCOL_NOT_SUPPORTED = 14,
    };

    
class CoroRpcTools
{
public:
    using rpc_func_key_t = uint32_t;
    using rpc_func_key = rpc_func_key_t;
    // 基础类型的序列化（带边界检查）
    template<typename T>
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, T &&arg)
    {
        // 确保缓冲区有足够空间
        if (offset + sizeof(T) > buffer.size()) {
            buffer.resize(offset + sizeof(T));
        }
        const T& value = arg;
        const std::byte*  p_bytes = reinterpret_cast<const std::byte*>(&value);
        std::copy(p_bytes, p_bytes + sizeof(T), buffer.begin() + offset);
        offset += sizeof(T);
    }

    // 字符串类型的特化版本（带边界检查）
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, std::string str)
    {
        // 先序列化字符串长度
        uint32_t length = static_cast<uint32_t>(str.size());
        SerializeArg(buffer, offset, length);

        // 确保 buffer 有足够的空间（包括字符串内容）
        if(offset + length > buffer.size())
        {
            buffer.resize(offset + length);
        }
        // 再序列化字符串内容
        const std::byte* bytes = reinterpret_cast<const std::byte*>(str.data());
        std::copy(bytes, bytes + length, buffer.begin() + offset);
        offset += length;
    }

    // 基础类型的反序列化（带边界检查）
    template<typename T>
    static T DeserializeArg(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        // 检查缓冲区边界
        if (offset + sizeof(T) > buffer.size()) {
            throw std::runtime_error("DeserializeArg: buffer overflow");
        }
        T value;
        std::byte* p_bytes = reinterpret_cast<std::byte*>(&value);
        std::copy(buffer.begin() + offset, buffer.begin() + offset + sizeof(T), p_bytes);
        offset += sizeof(T);
        return value;
    }

    // 字符串类型的反序列化特化版本（带边界检查）
    static std::string DeserializeString(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        // 先反序列化字符串长度
        uint32_t length = DeserializeArg<uint32_t>(buffer, offset);

        // 检查缓冲区边界
        if (offset + length > buffer.size()) {
            throw std::runtime_error("DeserializeString: buffer overflow");
        }
        // 反序列化字符串内容
        std::string result(reinterpret_cast<const char*>(buffer.data() + offset), length);
        offset += length;
        return result;
    }

    template<auto func>
    static constexpr rpc_func_key AutoGenRegisterKey()
    {
        constexpr auto name = ToolBox::GetFuncName<func>();
        auto id = MD5Hash32Constexpr(name);
        return static_cast<rpc_func_key>(id);
    }
};


// --------- 检查是否是 tuple_like 类型 ---------
template<typename T, typename = void>
struct is_tuple_like : std::false_type {};

template<typename T>
struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>> 
    : std::true_type {};

template<typename T>
inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;
// ---------------------------------------------


}   // namespace ToolBox::CoroRpc
