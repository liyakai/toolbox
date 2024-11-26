#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <string>
#include <format>

#include "tools/coro_rpc/impl/coro_rpc_def_interenal.h"
#include "struct_pack_protocol.h"


namespace ToolBox::CoroRpc
{
    
template <typename RpcProtocol, template<typename...> typename map_t = std::unordered_map>
class CoroRpcServer;
struct CoroRpcProtocol
{
public:
    constexpr static inline uint8_t VERSION_NUMBER = 0;
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
        uint8_t magic;
        uint8_t version;
        uint8_t serialize_type;
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
        uint8_t magic;
        uint8_t version;
        uint8_t err_code;
        uint8_t msg_type;
        uint32_t seq_num;
        uint32_t length;
        uint32_t attach_length;
    };

    using supported_serialize_protocols = std::variant<StructPackProtocol>;
    using rpc_server = CoroRpcServer<CoroRpcProtocol>;


    static std::optional<supported_serialize_protocols> GetSerializeProtocol(ReqHeader &header)
    {
        if(header.serialize_type == 0)
        {
            return StructPackProtocol();
        }else {
            return std::nullopt;
        }
    }
    static CoroRpcTools::rpc_func_key GenRpcFuncKey(const ReqHeader &header)
    {
        return header.func_id;
    }

    static Errc ReadHeader(std::string_view data, ReqHeader &header)
    {
        if(data.size() < REQ_HEAD_LEN)
        {
            RpcLogError("[CoroRpcProtocol] ReadHeader: data size is less than header length");
            return Errc::ERR_PROTOCOL;
        }
        std::memcpy(&header, data.data(), REQ_HEAD_LEN);
        if(header.length > UINT32_MAX || header.attach_length > UINT32_MAX)
        {
            RpcLogError("[CoroRpcProtocol] ReadHeader: length is too large");
            return Errc::ERR_PROTOCOL;
        }
        if (header.magic != magic_number) {
            RpcLogError("[CoroRpcProtocol] ReadHeader: magic number is invalid");
            return Errc::ERR_PROTOCOL;
        }
        if (header.version != VERSION_NUMBER) {
            RpcLogError("[CoroRpcProtocol] ReadHeader: version number is invalid");
            return Errc::ERR_PROTOCOL;
        }

        return Errc::SUCCESS;
    }

    static Errc ReadPayLoad(const ReqHeader &header, std::string_view data, std::string& body, std::string &attachment)
    {
        if(data.size() != header.length + header.attach_length)
        {
            RpcLogError("[CoroRpcProtocol] ReadPayLoad: data size is not equal to header length. data size: %zu, header length: %u, attach length: %u", data.size(), header.length, header.attach_length    );
            return Errc::ERR_PROTOCOL;
        }
        body.resize(header.length);
        std::memcpy(body.data(), data.data(), header.length);
        if(header.attach_length > 0)
        {
            attachment.resize(header.attach_length);
        }
        if(header.attach_length > 0)
        {
            std::memcpy(attachment.data(), data.data() + header.length, header.attach_length);
        }
        return Errc::SUCCESS;
    }

    template <auto func>
    struct HasGenRegisterKey {
        template <typename T>
        static constexpr bool check(T*) {
            return requires { T::template HasGenRegisterKey<func>(); };
        }
    };



    static bool PrepareResponseHeader(std::vector<std::byte> &response_buf, std::string& rpc_result, const ReqHeader& req_header, std::size_t attachment_len, CoroRpc::Errc err_code = {}, std::string_view err_msg = {})
    {
        std::string err_msg_buf;
        std::vector<std::byte>& header_buf = response_buf;
        if(header_buf.size() < RESP_HEAD_LEN)
        {
            RpcLogError("[CoroRpcProtocol] PrepareResponseHeader: response buffer is too small");
            return false;
        }
        auto& resp_head = *(RespHeader*)header_buf.data();
        resp_head.magic = magic_number;
        resp_head.version = VERSION_NUMBER;
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

    constexpr static inline uint8_t magic_number = 0xde;

    static constexpr inline auto REQ_HEAD_LEN = sizeof(ReqHeader);
    static_assert(REQ_HEAD_LEN == 20);
    static constexpr inline auto RESP_HEAD_LEN = sizeof(RespHeader);
    static_assert(RESP_HEAD_LEN == 16);

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