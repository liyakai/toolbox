#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <string>
#include <format>

#include "../coro_rpc_def_interenal.h"
#include "struct_pack_protocol.h"
#include "protobuf_protocol.h"
#include <google/protobuf/message.h>

namespace ToolBox::CoroRpc
{
    
template <typename RpcProtocol, template<typename...> typename map_t = std::unordered_map>
class CoroRpcServer;
struct CoroRpcProtocol
{
public:
    constexpr static inline uint8_t MAGIC_NUMBER = 0xde;
    constexpr static inline uint8_t VERSION_NUMBER = 1;
    constexpr static inline uint8_t SERIALIZE_TYPE = 1;
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
        uint8_t serialize_type = SERIALIZE_TYPE;
        uint8_t msg_type;
        uint32_t seq_num;
        uint32_t func_id;
        uint32_t length;
        uint32_t attach_length;
        std::string ToString()
        {
            return std::format("magic: {}, version: {}, serialize_type: {}, msg_type: {}, seq_num: {}, func_id: {}, length: {}, attach_length: {}",
                            magic, version, serialize_type, msg_type, seq_num, func_id, length, attach_length);
        }
    };

    struct RespHeader
    {
        uint8_t magic = MAGIC_NUMBER;
        uint8_t version = VERSION_NUMBER;
        uint8_t serialize_type = SERIALIZE_TYPE;
        uint8_t err_code;
        uint8_t msg_type;
        uint32_t seq_num;
        uint32_t length;
        uint32_t attach_length;
        std::string ToString()
        {
            return std::format("magic: {}, version: {}, serialize_type: {}, err_code: {}, msg_type: {}, seq_num: {}, length: {}, attach_length: {}",
                            magic, version, serialize_type, err_code, msg_type, seq_num, length, attach_length);
        }
    };

    using supported_serialize_protocols = std::variant<StructPackProtocol, ProtobufProtocol>;
    using rpc_server = CoroRpcServer<CoroRpcProtocol>;

    template<typename T>
    static std::optional<supported_serialize_protocols> GetSerializeProtocol(const T &header)
    {
        static_assert(std::is_same_v<T, ReqHeader> || std::is_same_v<T, RespHeader>, 
                  "T must be either ReqHeader or RespHeader");
        if(header.serialize_type == SERIALIZE_TYPE)
        {
            return ProtobufProtocol();
        }else {
            return std::nullopt;
        }
    }
    static std::optional<supported_serialize_protocols> GetSerializeProtocolByType(uint8_t serialize_type)
    {
        if(serialize_type == SERIALIZE_TYPE)
        {
            return ProtobufProtocol();
        }else {
            return std::nullopt;
        }
    }
    template<auto func, typename... Args>
    static supported_serialize_protocols GetSerializeProtocol()
    {
        if constexpr (sizeof...(Args) == 0) {
            return ProtobufProtocol();
        }
        else {
            using first_arg_type = std::tuple_element_t<0, std::tuple<Args...>>;
            if constexpr (std::is_base_of_v<::google::protobuf::Message, std::remove_cvref_t<first_arg_type>>) {
                return ProtobufProtocol();
            }
            else {
                return StructPackProtocol();
            }
        }
    }

    template<auto func>
    static CoroRpcTools::rpc_func_key GenRegisterKey()
    {
        // 使用函数指针的地址作为唯一标识符
        return CoroRpcTools::AutoGenRegisterKey<func>();;
    }
    
    static CoroRpcTools::rpc_func_key GetRpcFuncKey(const ReqHeader &header)
    {
        return header.func_id;
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

    // template <auto func>
    // struct HasGenRegisterKey {
    //     template <typename T>
    //     static constexpr bool check(T*) {
    //         return requires { T::template HasGenRegisterKey<func>(); };
    //     }
    // };



    static bool PrepareResponseHeader(std::string &response_buf, std::string& rpc_result, const ReqHeader& req_header, std::size_t attachment_len, CoroRpc::Errc err_code = {}, std::string_view err_msg = {})
    {
        std::string err_msg_buf;
        std::string& header_buf = response_buf;
        if(header_buf.capacity() < RESP_HEAD_LEN)
        {
            RpcLogError("[CoroRpcProtocol] PrepareResponseHeader: response buffer is too small, size: %zu, expected: %zu", header_buf.size(), RESP_HEAD_LEN);
            return false;
        }
        auto& resp_head = *(RespHeader*)header_buf.data();
        resp_head.magic = MAGIC_NUMBER;
        resp_head.version = VERSION_NUMBER;
        resp_head.serialize_type = SERIALIZE_TYPE;
        resp_head.seq_num = req_header.seq_num;
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
    static_assert(REQ_HEAD_LEN == 20);
    static constexpr inline auto RESP_HEAD_LEN = sizeof(RespHeader);
    static_assert(RESP_HEAD_LEN == 20);

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