#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <string>
#include <format>
#include <utility>
#include <tuple>

#include "../coro_rpc_def_interenal.h"
#include "struct_pack_protocol.h"
#include "protobuf_protocol.h"
#include "tools/function_traits.h"
#include <google/protobuf/message.h>

namespace ToolBox::CoroRpc
{
    // Concepts 增强类型安全
    template<typename T>
    concept IsProtobufMessage = std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<T>>;
    
    // 检查函数是否适用于 ProtobufProtocol
    template<auto func>
    struct IsProtobufCompatible {
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using return_type = typename traits_type::return_type;
        
        static constexpr bool value = []() {
            // 检查返回值类型
            if constexpr (!std::is_void_v<return_type>) {
                if constexpr (!IsProtobufMessage<return_type>) {
                    return false;
                }
            }
            
            // 检查所有参数类型
            if constexpr (std::is_void_v<param_type> || std::tuple_size_v<param_type> == 0) {
                // 无参数函数，只检查返回值
                return std::is_void_v<return_type> || IsProtobufMessage<return_type>;
            } else {
                // 检查所有参数是否都是 protobuf 消息类型
                return []<std::size_t... I>(std::index_sequence<I...>) {
                    return (IsProtobufMessage<std::tuple_element_t<I, param_type>> && ...);
                }(std::make_index_sequence<std::tuple_size_v<param_type>>{}) 
                    && (std::is_void_v<return_type> || IsProtobufMessage<return_type>);
            }
        }();
    };
    
    template<auto func>
    inline constexpr bool IsProtobufCompatible_v = IsProtobufCompatible<func>::value;
    
template <typename RpcProtocol, template<typename...> typename map_t = std::unordered_map>
class CoroRpcServer;
struct CoroRpcProtocol
{
public:
    constexpr static inline uint8_t MAGIC_NUMBER = 0xde;
    constexpr static inline uint8_t VERSION_NUMBER = 1;
    
    // 序列化类型枚举
    enum class SerializeType : uint8_t {
        SERIALIZE_TYPE_STRUCT = 0,    // StructPack 序列化方式（用于基本类型）
        SERIALIZE_TYPE_PROTOBUF = 1   // Protobuf 序列化方式（用于 protobuf message）
    };
/*
* RPC header
* 
* memory layout:
* ```
* ┌────────┬───────────┬────────────────┬──────────┬─────────┬─────────┐─────────┐
* │  magic │  version  │ serialize_type │ msg_type │ seq_num │ length
* │reserved
* ├────────┼───────────┼────────────────┼──────────┼─────────┼─────────┤─────────┤
* │   1    │     1     │       1        │    1     │    4    │    4    │    4
* │
* └────────┴───────────┴────────────────┴──────────┴─────────┴─────────┘─────────┘
* ```
*/
    struct ReqHeader
    {
        uint8_t magic = MAGIC_NUMBER;
        uint8_t version = VERSION_NUMBER;
        uint8_t serialize_type = static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_STRUCT);  // 默认使用 StructPack
        uint8_t msg_type;
        uint32_t seq_num;
        uint32_t func_id;
        uint32_t length;
        uint64_t client_id;
        uint32_t attach_length;
        std::string ToString() const
        {
            return std::format("magic: {}, version: {}, serialize_type: {}, msg_type: {}, seq_num: {}, func_id: {}, client_id:{}, length: {}, attach_length: {}",
                            magic, version, serialize_type, msg_type, seq_num, func_id, client_id, length, attach_length);
        }
    };

    struct RespHeader
    {
        uint8_t magic = MAGIC_NUMBER;
        uint8_t version = VERSION_NUMBER;
        uint8_t serialize_type = static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_STRUCT);  // 默认使用 StructPack
        uint8_t err_code;
        uint8_t msg_type;
        uint32_t seq_num;
        uint64_t client_id;
        uint32_t length;
        uint32_t attach_length;
        std::string ToString() const
        {
            return std::format("magic: {}, version: {}, serialize_type: {}, err_code: {}, msg_type: {}, seq_num: {}, client_id:{}, length: {}, attach_length: {}",
                            magic, version, serialize_type, err_code, msg_type, seq_num, client_id, length, attach_length);
        }
    };

    using supported_serialize_protocols = std::variant<StructPackProtocol, ProtobufProtocol>;
    using rpc_server = CoroRpcServer<CoroRpcProtocol>;

    template<typename T>
    static std::optional<supported_serialize_protocols> GetSerializeProtocol(const T &header)
    {
        static_assert(std::is_same_v<T, ReqHeader> || std::is_same_v<T, RespHeader>, 
                  "T must be either ReqHeader or RespHeader");
        if(header.serialize_type == static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_PROTOBUF))
        {
            return ProtobufProtocol();
        }else if(header.serialize_type == static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_STRUCT))
        {
            return StructPackProtocol();
        }else {
            // 未知的序列化类型
            return std::nullopt;
        }
    }
    static std::optional<supported_serialize_protocols> GetSerializeProtocolByType(uint8_t serialize_type)
    {
        if(serialize_type == static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_PROTOBUF))
        {
            return ProtobufProtocol();
        }else if(serialize_type == static_cast<uint8_t>(SerializeType::SERIALIZE_TYPE_STRUCT))
        {
            return StructPackProtocol();
        }else {
            // 未知的序列化类型
            return std::nullopt;
        }
    }
    template<auto func>
    static supported_serialize_protocols GetSerializeProtocol()
    {
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using return_type = typename traits_type::return_type;
        
        // 检查返回值类型
        constexpr bool return_type_is_protobuf = []() {
            if constexpr (std::is_void_v<return_type>) {
                return true;  // void 返回值视为兼容
            } else {
                return IsProtobufMessage<return_type>;
            }
        }();
        
        // 检查所有参数类型
        constexpr bool all_params_are_protobuf = []() {
            if constexpr (std::is_void_v<param_type> || std::tuple_size_v<param_type> == 0) {
                // 无参数函数
                return true;
            } else {
                // 检查所有参数是否都是 protobuf 消息类型
                return []<std::size_t... I>(std::index_sequence<I...>) {
                    return (IsProtobufMessage<std::tuple_element_t<I, param_type>> && ...);
                }(std::make_index_sequence<std::tuple_size_v<param_type>>{});
            }
        }();
        
        // 只有当所有参数和返回值都是 protobuf 消息类型时，才使用 ProtobufProtocol
        if constexpr (all_params_are_protobuf && return_type_is_protobuf) {
            return ProtobufProtocol();
        } else {
            return StructPackProtocol();
        }
    }

    template<auto func>
    static constexpr CoroRpcTools::rpc_func_key GenRegisterKey()
    {
        // 使用函数指针的地址作为唯一标识符
        return CoroRpcTools::AutoGenRegisterKey<func>();
    }
    
    static CoroRpcTools::rpc_func_key GetRpcFuncKey(const ReqHeader &header)
    {
        return header.func_id;
    }

    static uint64_t GetClientID(const ReqHeader &header)
    {
        return header.client_id;
    }

    template<typename T>
    static Errc ReadHeader(std::string_view data, T &header)
    {
        const size_t size = sizeof(T);
        if(data.size() < size)
        {
            RpcLogError("[CoroRpcProtocol] ReadHeader: data size is less than header length, data size: %zu, header length: %zu", data.size(), size);
            return Errc::ERR_PROTOCOL;
        }
        // Check alignment for memcpy
        if (reinterpret_cast<std::uintptr_t>(data.data()) % alignof(T) != 0) {
            // If not aligned, use byte-by-byte copy
            std::memcpy(&header, data.data(), size);
        } else {
            header = *reinterpret_cast<const T*>(data.data());
        }
        
        if(header.length > UINT32_MAX || header.attach_length > UINT32_MAX)
        {
            RpcLogError("[CoroRpcProtocol] ReadHeader: length is too large");
            return Errc::ERR_PROTOCOL;
        }
        if (header.magic != MAGIC_NUMBER) {
            RpcLogError("[CoroRpcProtocol] ReadHeader: magic number is invalid, magic: %d, expected: %d", header.magic, MAGIC_NUMBER);
            return Errc::ERR_PROTOCOL;
        }
        if (header.version != VERSION_NUMBER) {
            RpcLogError("[CoroRpcProtocol] ReadHeader: version number is invalid, version: %d, expected: %d", header.version, VERSION_NUMBER);
            return Errc::ERR_PROTOCOL;
        }

        return Errc::SUCCESS;
    }

    template<typename T>
    static Errc ReadPayLoad(const T &header, std::string_view data, std::string_view &body, std::string_view &attachment)
    {
        if(data.size() != header.length + header.attach_length)
        {
            RpcLogError("[CoroRpcProtocol] ReadPayLoad: data size is not equal to header length. data size: %zu, header length: %u, attach length: %u", data.size(), header.length, header.attach_length    );
            return Errc::ERR_PROTOCOL;
        }
        body = std::string_view(data.data(), header.length);
        if(header.attach_length > 0)
        {
            attachment = std::string_view(data.data() + header.length, header.attach_length);
        }
        return Errc::SUCCESS;
    }

    static bool PrepareResponseHeader(std::string &response_buf, const std::string& rpc_result, const ReqHeader& req_header, std::size_t attachment_len, CoroRpc::Errc err_code = {}, std::string_view err_msg = {})
    {
        std::string& header_buf = response_buf;
        // 确保缓冲区有足够空间
        if(header_buf.size() < RESP_HEAD_LEN)
        {
            header_buf.resize(RESP_HEAD_LEN);
        }
        auto& resp_head = *reinterpret_cast<RespHeader*>(header_buf.data());
        resp_head.magic = MAGIC_NUMBER;
        resp_head.version = VERSION_NUMBER;
        // 响应使用与请求相同的序列化类型
        resp_head.serialize_type = req_header.serialize_type;
        resp_head.seq_num = req_header.seq_num;
        resp_head.client_id = req_header.client_id;
        resp_head.attach_length = attachment_len; 
        resp_head.length = rpc_result.size();

        if (attachment_len > UINT32_MAX) 
        [[unlikely]] {
            err_code = CoroRpc::Errc::ERR_MESSAGE_TOO_LARGE;
        }else if (rpc_result.size() > UINT32_MAX) 
        [[unlikely]] {
            err_code = CoroRpc::Errc::ERR_MESSAGE_TOO_LARGE;
        }
        resp_head.err_code = static_cast<uint8_t>(err_code);
        return true;
    }

    static constexpr inline auto REQ_HEAD_LEN = sizeof(ReqHeader);
    static_assert(REQ_HEAD_LEN == 32);
    static constexpr inline auto RESP_HEAD_LEN = sizeof(RespHeader);
    static_assert(RESP_HEAD_LEN == 32);

}; // namespace ToolBox::CoroRpc::CoroRpcProtocol

template<typename RpcProtocol = CoroRpcProtocol>
static constexpr uint64_t GetRequestId(const typename RpcProtocol::ReqHeader& header)
{
    if constexpr (std::is_same_v<RpcProtocol, CoroRpcProtocol>)
    {
        return header.seq_num;
    }else {
        return 0;
    }
}

} // namespace ToolBox::CoroRpc