#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <string>

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
using rpc_server_key_t = uint32_t;
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

template <auto func>
struct HasGenRegisterKey {
    template <typename T>
    static constexpr bool check(T*) {
        return requires { T::template HasGenRegisterKey<func>(); };
    }
};

static std::string PrepareResponse(std::string& rpc_result, const RespHeader& req_header, std::size_t attachment_len, CoroRpc::Errc err_code = {}, std::string_view err_msg = {})
{
    std::string err_msg_buf;
    std::string header_buf;
    header_buf.resize(RESP_HEAD_LEN);
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
    resp_head.length = rpc_result.size();
    return header_buf;
}

constexpr static inline uint8_t magic_number = 0xde;

static constexpr inline auto REQ_HEAD_LEN = sizeof(ReqHeader);
static_assert(REQ_HEAD_LEN == 20);
static constexpr inline auto RESP_HEAD_LEN = sizeof(RespHeader);
static_assert(RESP_HEAD_LEN == 16);

}; // namespace ToolBox::CoroRpc::CoroRpcProtocol

template<typename RpcProtocol = CoroRpcProtocol>
uint64_t GetRequestId(const typename RpcProtocol::ReqHeader& header)
{
    if constexpr (std::is_same_v<RpcProtocol, CoroRpcProtocol>)
    {
        return header.seq_num;
    }else {
        return 0;
    }
}

} // namespace ToolBox::CoroRpc