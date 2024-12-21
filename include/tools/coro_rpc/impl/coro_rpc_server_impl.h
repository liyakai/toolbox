#pragma once

#include "coro_rpc_def_interenal.h"
#include "protocol/coro_rpc_protocol.h"
#include "tools/cpp20_coroutine.h"
#include "tools/function_traits.h"
namespace ToolBox {
namespace CoroRpc {

template<typename T, auto func>
concept HasGenRegisterKey = requires() {
    T::template GenRegisterKey<func>();
};

template <typename rpc_protocol, template<typename...> typename map_t>
class CoroRpcServer {
public:
    using handler_t = std::function<std::pair<Errc, std::string>(std::string_view, typename rpc_protocol::supported_serialize_protocols)>;
    using core_handler_t = std::function<ToolBox::coro::Task<std::pair<Errc, std::string>>(std::string_view, typename rpc_protocol::supported_serialize_protocols protocols)>;
    using rpc_func_key = typename CoroRpcTools::rpc_func_key_t;
private:
    using SendCallback = std::function<void(std::vector<std::byte> &&)>;
    SendCallback send_callback_ = nullptr;
    std::unordered_map<rpc_func_key, handler_t> rpc_server_handler_map_;
    std::unordered_map<rpc_func_key, core_handler_t> rpc_server_core_handler_map_;
    std::unordered_map<rpc_func_key, std::string> id2name_map_;
    std::unordered_map<rpc_func_key, std::function<std::string_view()>> resp_attachment_func_map_;
public:
    CoroRpcServer() = default;
    ~CoroRpcServer() = default;

    /*
    * @brief Register RPC service functions(member function)
    */
    template <auto first, auto... functions>
    void RegisterService(decltype(first) *self) {
        RegisterOneHandler<first>(self);
        (RegisterOneHandler<functions>(self), ...);
    }

    template <auto first>
    void RegisterService(decltype(first) *self, const auto &key) {
        RegistOneHandlerImpl<first>(self, key);
    }

    /*
    * @brief Register RPC service functions (non-member function)
    */
    template <auto first, auto... func>
    void RegisterService() {
        RegisterOneHandler<first>();
        (RegisterOneHandler<func>(), ...);
    }

    template <auto func>
    void RegisterService(const auto &key) {
        RegistOneHandlerImpl<func>(key);
    }

    const std::string& GetName(const rpc_func_key &key) const {
        static std::string empty_str;
        auto it = id2name_map_.find(key);
        if (it == id2name_map_.end()) {
            return empty_str;
        }
        return it->second;
    }

    Errc OnRecvReq(std::string_view data) {
        return OnRecvReq_(data);
    }
    Errc SetSendCallback(std::function<void(std::vector<std::byte> &&)> &&callback) {
        send_callback_ = std::move(callback);
        return Errc::SUCCESS;
    }
    template <auto func>
    void SetRespAttachmentFunc(std::function<std::string_view()> &&resp_attachment_func) {
        rpc_func_key key{};
        if constexpr(HasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }
        resp_attachment_func_map_[key] = std::move(resp_attachment_func);
    }
private:
    template <auto func, typename Self>
    void RegisterOneHandler(Self *self)
    {
        if (nullptr == self) [[unlikely]]
        {
            RpcLogError("[CoroRpcServer] RegisterOneHandler: self is nullptr");
            return;
        }
        rpc_func_key key{};
        if constexpr(HasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }

        RpcLogDebug("[rpc][server] RegisterOneHandler.class: key: %u, func: %s", key, ToolBox::GetFuncName<func>().c_str());

        RegistOneHandlerImpl<func>(self,key);
    }


    template<auto func, typename Self>
    void RegistOneHandlerImpl(Self *self, const rpc_func_key &key)
    {
        if (self == nullptr) [[unlikely]]
        {
            RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: self is nullptr");
            return;
        }
        using ReturnType = std::invoke_result_t<decltype(func)>;
        constexpr auto name = ToolBox::GetFuncName<func>();

        if constexpr(ToolBox::is_specialization_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return ExecuteCore_<rpc_protocol, typename rpc_protocol::supported_serialize_protocols, func>(data, self);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return Execute_<typename rpc_protocol::supported_serialize_protocols, func>(data, self);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        }
    }

    template <auto func>
    void RegisterOneHandler()
    {
        rpc_func_key key{};
        if constexpr(HasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }
        RpcLogDebug("[rpc][server] RegisterOneHandler.normal: key: %u, func: %s", key, ToolBox::GetFuncName<func>().data());
        RegistOneHandlerImpl<func>(key);
    }

    template<auto func>
    void RegistOneHandlerImpl(const rpc_func_key &key)
    {
        static_assert(!std::is_member_function_pointer_v<decltype(func)>, "RegistOneHandlerImpl: func is not a valid function");
        using ReturnType = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        if constexpr(ToolBox::is_specialization_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    return ExecuteCore_<rpc_protocol, serialize_protocol, func>(data);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    return Execute_<serialize_protocol, func>(data);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        }
    }

    handler_t GetHandler(const rpc_func_key &key)
    {
        auto iter = rpc_server_handler_map_.find(key);
        if(iter == rpc_server_handler_map_.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    core_handler_t GetCoreHandler(const rpc_func_key &key)
    {
        auto iter = rpc_server_core_handler_map_.find(key);
        if(iter == rpc_server_core_handler_map_.end())
        {
            return nullptr;
        }
        return iter->second;
    }

    ToolBox::coro::Task<std::pair<Errc, std::string>> HandleCoro(auto handler, std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols, const typename CoroRpcTools::rpc_func_key_t &key)
    {
        using namespace std::string_literals;
        if(handler) [[likely]]
        {
            try {
                co_return co_await handler(data, protocols);
            } catch(Errc &err)
            {
                RpcLogError("[CoroRpcServer] HandleCoro: handler error, err:%d", err);
                co_return std::make_pair(err, ""); 
            } catch (const std::exception &e) {
                RpcLogError("[CoroRpcServer] HandleCoro: exception: %s", e.what());
                co_return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            } catch(...)
            {
                RpcLogError("[CoroRpcServer] HandleCoro: unknown exception");
                co_return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            }
        }else {
            RpcLogError("[CoroRpcServer] HandleCoro: handler not found");
            co_return std::make_pair(Errc::ERR_FUNC_NOT_REGISTERED, "");
        }
    }

    std::pair<Errc, std::string> Handle(auto handler, std::string_view payload, typename rpc_protocol::supported_serialize_protocols protocols)
    {
        using namespace std::string_literals;
        if(handler) [[likely]]
        {
            try {
                return handler(payload, protocols);
            } catch(Errc &err)
            {
                RpcLogError("[CoroRpcServer] Handle: handler error, err:%d", err);
                return std::make_pair(err, ""); 
            } catch (const std::exception &e) {
                RpcLogError("[CoroRpcServer] Handle: exception: %s", e.what());
                return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "");
            } catch(...)
            {
                RpcLogError("[CoroRpcServer] Handle: unknown exception");
                return std::make_pair(Errc::ERR_HANDLER_THROW_EXCEPTION, "")    ;
            }
        }else {
            RpcLogError("[CoroRpcServer] Handle: handler not found");
            return std::make_pair(Errc::ERR_FUNC_NOT_REGISTERED, "");
        }
    }

    template<typename serialize_proto, auto func, typename Self = void>
    inline std::pair<Errc, std::string> Execute_(std::string_view data, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using ReturnType = typename traits_type::return_type;
        fprintf(stderr, "coro_rpc server Execute_, func type: %s, data size: %zu, data: %s\n", ToolBox::GetFuncName<func>().data(), data.size(), data.data());
        if constexpr(!std::is_void_v<param_type>)
        {
            using First = std::tuple_element_t<0, param_type>;
            param_type args;
            bool is_ok = true;
            constexpr size_t size = std::tuple_size_v<param_type>;
            if constexpr(size > 0)
            {
                is_ok = serialize_proto::Deserialize(args, data);
            }
            if (!is_ok)[[unlikely]]
            {
                RpcLogError("[CoroRpcServer] Execute_: deserialize arguments failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            if constexpr (std::is_void_v<ReturnType>)
            {
                if constexpr (std::is_void_v<Self>)
                {
                    std::apply(func, std::move(args));
                } else {
                    std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)));
                }
                return std::make_pair(Errc::SUCCESS, serialize_proto::Serialize());
            } else {
                if constexpr (std::is_void_v<Self>)
                {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::Serialize(std::apply(func, std::move(args))));
                } else {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::Serialize(std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)))));
                }
            }
        }else {
            if constexpr(std::is_void_v<ReturnType>)
            {
                if constexpr(std::is_void_v<Self>)
                {
                    func();
                } else {
                    std::apply(func, std::tuple_cat(std::forward_as_tuple(*self)));
                }
            } else {
                if constexpr(std::is_void_v<Self>)
                {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::Serialize(func()));
                }else {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::Serialize(std::apply(func, std::tuple_cat(std::forward_as_tuple(*self)))));
                }
            }
        }
    }

    template<typename serialize_proto, auto func, typename Self = void>
    inline coro::Task<std::pair<Errc, std::string>> ExecuteCore_(std::string_view data, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        // 定义参数类型
        using param_type = std::conditional_t<
            std::is_member_function_pointer_v<func_type>,
            typename ToolBox::FunctionTraits<func_type>::template args_tuple<1>,  // 成员函数：跳过第一个this参数
            typename ToolBox::FunctionTraits<func_type>::args_tuple              // 普通函数：使用全部参数
        >;
        using ReturnType = std::invoke_result_t<decltype(func)>;
        fprintf(stderr, "coro_rpc server ExecuteCore_, func type: %s\n", ToolBox::GetFuncName<func>().data());
        if constexpr(!std::is_void_v<param_type>){
            using First = std::tuple_element_t<0, param_type>;
            bool is_ok = true;
            constexpr size_t size = std::tuple_size_v<param_type>;
            param_type args;
            if constexpr(size > 0)
            {
                is_ok = serialize_proto::Deserialize(args, data);
            }
            if (!is_ok)[[unlikely]]
            {
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            if constexpr (std::is_void_v<ReturnType>)
            {
                if constexpr(std::is_void_v<Self>)
                {
                    // call void func(args...)
                    co_await std::apply(func, std::move(args));
                } else {
                    // call void self->func(self, args...)
                    co_await std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)));
                }
                co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(std::monostate{}));
            } else {
                if constexpr (std::is_void_v<Self>)
                {
                    // call return_type func(args...)
                    co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(co_await std::apply(func, std::move(args))));
                } else {
                    // call return_type self->func(self, args...)
                    co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(co_await std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)))));
                }
            }
        } else {
            if constexpr (std::is_void_v<ReturnType>)
            {
                if constexpr(std::is_void_v<Self>)
                {
                    co_await func();
                } else {
                    co_await std::apply(func, std::tuple_cat(std::forward_as_tuple(*self)));
                }
                co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(std::monostate{}));
            } else {
                if constexpr(std::is_void_v<Self>)
                {
                    co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(co_await func()));
                } else {
                    co_return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(co_await std::apply(func, std::tuple_cat(std::forward_as_tuple(*self)))));
                }
            }
        }
    }   // ExecuteCore_
    Errc OnRecvReq_(std::string_view data)
    {
        typename rpc_protocol::ReqHeader header;
        auto err = rpc_protocol::ReadHeader(data, header);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read header failed, err:%d", err);
            return err;
        }
        fprintf(stderr, "coro_rpc server recv data[size:%zu], header: %s\n", data.size(), header.ToString().c_str());

        auto serialize_protocol = rpc_protocol::GetSerializeProtocol(header);
        if(!serialize_protocol.has_value())
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: serialize protocol not supported");
            return Errc::ERR_PROTOCOL;
        }

        std::string_view payload;
        std::string_view attachment;
        err = rpc_protocol::ReadPayLoad(header, data.substr(rpc_protocol::REQ_HEAD_LEN), payload, attachment);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read payload failed, err: {}", err);
            return err;
        }

        Errc resp_err = Errc::SUCCESS;
        std::string rpc_result;
        auto key = rpc_protocol::GenRpcFuncKey(header);
        auto handler = GetHandler(key);
        if(handler == nullptr)
        {
            auto core_handler = GetCoreHandler(key);
            auto handler_ret = HandleCoro(core_handler, payload, serialize_protocol.value(), key);
            std::tie(resp_err, rpc_result) = handler_ret.get_result();
            if(resp_err != Errc::SUCCESS)
            {
                RpcLogError("[CoroRpcServer] OnRecvReq: handler error, err:%d", resp_err);
                return resp_err;
            }
        }else {
            std::tie(resp_err, rpc_result) = Handle(handler, payload, serialize_protocol.value());
            if(resp_err != Errc::SUCCESS)
            {
                RpcLogError("[CoroRpcServer] OnRecvReq: handler error, err:%d", resp_err);
                return resp_err;
            }
        }
        Errc ret = Errc::SUCCESS;
        auto resp_attachment_func_iter = resp_attachment_func_map_.find(key);
        if(resp_attachment_func_iter != resp_attachment_func_map_.end())
        {
            ret = DirectResponseMsg(resp_err, rpc_result, header, std::move(resp_attachment_func_iter->second));
            resp_attachment_func_map_.erase(resp_attachment_func_iter);
        } else {
            ret = DirectResponseMsg(resp_err, rpc_result, header);
        }
        if(ret != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: prepare response header failed");
            return ret;
        }

        return Errc::SUCCESS;
    }
    Errc DirectResponseMsg(Errc resp_err, std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head) {
        return DirectResponseMsg(resp_err, rpc_result, req_head, 
                               [](){ return std::string_view{}; });
    }
    Errc DirectResponseMsg(Errc resp_err, std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head,
        std::function<std::string_view()> &&attachment_func)
    {
        std::string resp_error_msg;
        if(resp_err != Errc::SUCCESS)
        {
            resp_error_msg = std::move(rpc_result);
            rpc_result = {};
            RpcLogWarn("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", resp_err);
        }
        std::string_view attachment = attachment_func();
        std::vector<std::byte> resp_buf_bytes(rpc_protocol::RESP_HEAD_LEN + rpc_result.size() + attachment.length());
        RpcLogDebug("[rpc][server] DirectResPonseMsg.resp_buf_bytes: resp_buf_bytes size: %zu, rpc_result: %zu, attachment_size: %zu", resp_buf_bytes.size(), rpc_result.size(), attachment.length());
        bool prepare_ret = rpc_protocol::PrepareResponseHeader(resp_buf_bytes, rpc_result, req_head, attachment.length(), resp_err, resp_error_msg);
        if(!prepare_ret)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: prepare response header failed");
            return Errc::ERR_SERVER_PREPARE_HEADER_FAILED;
        }
        auto response_ret = Response(std::move(resp_buf_bytes), std::move(rpc_result), std::move(attachment));
        Errc err_code = response_ret.get_result();
        if(err_code != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", err_code);
            return err_code;
        }
        return Errc::SUCCESS;
    }
    ToolBox::coro::Task<Errc> Response(std::vector<std::byte> &&resp_buf_bytes, std::string &&rpc_result, std::string_view &&attachment)
    {
        if(!send_callback_)
        {
            co_return Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN, rpc_result.data(), rpc_result.size());
        if(attachment.length() > 0)
        {
            std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN + rpc_result.size(), attachment.data(), attachment.size());
        }
        send_callback_(std::move(resp_buf_bytes));
        co_return Errc::SUCCESS;
    }
};  // class CoroRpcServer

}   // namespace ToolBox::CoroRpc
}   // namespace ToolBox