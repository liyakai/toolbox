#pragma once

#include "coro_rpc_def_interenal.h"
#include "protocol/coro_rpc_protocol.h"
#include "tools/cpp20_coroutine.h"
#include "tools/function_name.h"
#include "tools/md5.h"
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
    using handler_t = std::function<std::pair<Errc, std::string>(std::string_view, typename CoroRpcProtocol::supported_serialize_protocols)>;
    using core_handler_t = std::function<ToolBox::coro::Task<std::pair<Errc, std::string>>(std::string_view, typename CoroRpcProtocol::supported_serialize_protocols protocols)>;
    using rpc_server_key = typename rpc_protocol::rpc_server_key_t;
private:
    std::unordered_map<rpc_server_key, handler_t> rpc_server_handler_map_;
    std::unordered_map<rpc_server_key, core_handler_t> rpc_server_core_handler_map_;
    std::unordered_map<rpc_server_key, std::string> id2name_map_;

public:
    CoroRpcServer() = default;
    ~CoroRpcServer() = default;

    /*
    * @brief Register RPC service functions(member function)
    */
    template <auto first, auto... functions>
    void RegisterService(decltype(first) *self) {
        RegisterService<first, functions...>(self);
    }

    template <auto first>
    void RegisterService(decltype(first) *self, const auto &key) {
        RegisterService<first>(self, key);
    }

    /*
    * @brief Register RPC service functions (non-member function)
    */
    template <auto... functions>
    void RegisterService() {
        RegisterService<functions...>();
    }

    template <auto func>
    void RegisterService(const auto &key) {
        RegisterService<func>(key);
    }

    const std::string& GetName(const rpc_server_key &key) const {
        static std::string empty_str;
        auto it = id2name_map_.find(key);
        if (it == id2name_map_.end()) {
            return empty_str;
        }
        return it->second;
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
        rpc_server_key key{};
        if constexpr(HasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = AutoGenRegisterKey<func>();
        }

        RegistOneHandlerImpl<func>(self,key);
    }

    template<auto func>
    static rpc_server_key AutoGenRegisterKey()
    {
        constexpr auto name = ToolBox::GetFuncName<func>();
        ToolBox::MD5 md5(name.data());
        auto id = md5.toStr();
        return static_cast<rpc_server_key>(id);
    }
    template<auto func, typename Self>
    void RegistOneHandlerImpl(Self *self, const rpc_server_key &key)
    {
        if (self == nullptr) [[unlikely]]
        {
            RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: self is nullptr");
            return;
        }
        using ReturnType = std::invoke_result_t<decltype(func)>;
        constexpr auto name = ToolBox::GetFuncName<func>();

        if constexpr(ToolBox::CoroRpc::is_specialization_of_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return ExecuteCore_<rpc_protocol, rpc_protocol::supported_serialize_protocols, func>(data, self);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return Execute_<rpc_protocol, protocols, func>(data, self);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        }
    }

    template<typename exe_rpc_protocol, typename serialize_proto, auto func, typename Self = void>
    inline std::pair<Errc, std::string> Execute_(std::string_view data, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using param_type = std::conditional_t<
            std::is_member_function_pointer_v<func_type>,
            typename ToolBox::function_traits<func_type>::template args_tuple<1>,  // 成员函数：跳过第一个this参数
            typename ToolBox::function_traits<func_type>::args_tuple              // 普通函数：使用全部参数
        >;
        using ReturnType = std::invoke_result_t<decltype(func)>;
        if constexpr(!std::is_void_v<param_type>)
        {
            using First = std::tuple_element_t<0, param_type>;
            param_type args;
            bool is_ok = true;
            constexpr size_t size = std::tuple_size_v<param_type>;
            if constexpr(size > 0)
            {
                is_ok = serialize_proto::DeserializeArg(data, args);
            }
            if (!is_ok)[[unlikely]]
            {
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
            } else {
                if constexpr (std::is_void_v<Self>)
                {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(std::apply(func, std::move(args))));
                } else {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)))));
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
                    return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(func()));
                }else {
                    return std::make_pair(Errc::SUCCESS, serialize_proto::serialize(std::apply(func, std::tuple_cat(std::forward_as_tuple(*self)))));
                }
            }
        }
    }

    template<typename exe_rpc_protocol, typename serialize_proto, auto func, typename Self = void>
    inline coro::Task<std::pair<Errc, std::string>> ExecuteCore_(std::string_view data, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        // 定义参数类型
        using param_type = std::conditional_t<
            std::is_member_function_pointer_v<func_type>,
            typename ToolBox::function_traits<func_type>::template args_tuple<1>,  // 成员函数：跳过第一个this参数
            typename ToolBox::function_traits<func_type>::args_tuple              // 普通函数：使用全部参数
        >;
        using ReturnType = std::invoke_result_t<decltype(func)>;
        if constexpr(!std::is_void_v<param_type>){
            using First = std::tuple_element_t<0, param_type>;
            bool is_ok = true;
            constexpr size_t size = std::tuple_size_v<param_type>;
            param_type args;
            if constexpr(size > 0)
            {
                is_ok = serialize_proto::DeserializeArg(data, args);
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
};  // class CoroRpcServer

}   // namespace ToolBox::CoroRpc
}   // namespace ToolBox