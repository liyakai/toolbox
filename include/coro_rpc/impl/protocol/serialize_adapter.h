#pragma once

#include "coro_rpc_protocol.h"
#include "protobuf_protocol.h"
#include "struct_pack_protocol.h"
#include "../coro_rpc_def_interenal.h"

namespace ToolBox::CoroRpc {

/**
 * @brief 统一的序列化适配器
 * 
 * 提供统一的接口来访问所有协议类型的序列化操作，隐藏协议特定的实现细节。
 * 这个适配器层使得客户端和服务器端可以使用相同的代码来处理不同协议类型。
 */
template<auto func, typename serialize_protocol>
struct SerializeAdapter {
    // 获取序列化类型枚举值
    static constexpr uint8_t get_serialize_type() {
        if constexpr (std::is_same_v<serialize_protocol, ProtobufProtocol>) {
            return static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeProtobuf);
        } else if constexpr (std::is_same_v<serialize_protocol, StructPackProtocol>) {
            return static_cast<uint8_t>(CoroRpcProtocol::SerializeType::kSerializeTypeStruct);
        } else {
            // 未来扩展其他协议类型时可以在这里添加
            // 使用 std::false_type 来避免在 C++20 中立即触发 static_assert
            static_assert(std::is_same_v<serialize_protocol, serialize_protocol> && false, 
                         "Unknown serialize protocol type");
            return static_cast<uint8_t>(0);
        }
    }
    
    // 检查协议兼容性
    static constexpr bool is_compatible() {
        if constexpr (std::is_same_v<serialize_protocol, ProtobufProtocol>) {
            return IsProtobufCompatible_v<func>;
        } else {
            return true;  // 其他协议类型总是兼容
        }
    }
    
    // 统一的序列化大小计算
    template<typename... Args>
    static auto serialize_size(Args &&...args) -> uint32_t {
        // 使用 if constexpr 避免编译错误（ProtobufProtocol 对非 protobuf 类型会编译失败）
        if constexpr (!is_compatible()) {
            return static_cast<uint32_t>(0);  // 理论上不会执行到这里
        } else {
            return serialize_protocol::SerializeSize(std::forward<Args>(args)...);
        }
    }
    
    // 统一的序列化到缓冲区
    template<typename... Args>
    static auto serialize_to_buffer(void* data, int size, Args &&...args) -> Errc {
        // 使用 if constexpr 避免编译错误
        if constexpr (!is_compatible()) {
            return Errc::ERR_SERIALIZE_FAILED;  // 理论上不会执行到这里
        } else {
            return serialize_protocol::SerializeToBuffer(data, size, std::forward<Args>(args)...);
        }
    }
    
    // 统一的序列化到字符串
    template<typename T>
    static auto serialize(const T& value) -> std::string {
        if constexpr (!is_compatible()) {
            return std::string{};  // 理论上不会执行到这里
        } else {
            return serialize_protocol::Serialize(value);
        }
    }
    
    // 无参数序列化（用于 void 返回值）
    static auto serialize() -> std::string {
        return serialize_protocol::Serialize();
    }
    
    // 统一的反序列化
    template<typename T>
    static auto deserialize(T& value, std::string_view buffer) -> bool {
        return serialize_protocol::Deserialize(value, buffer);
    }
};

} // namespace ToolBox::CoroRpc

