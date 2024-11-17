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

       auto iter = rpc_server_handler_map_.emplace(key, [self](std::string_view data, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
            return (self->*func)(data, protocols);
       });
       if (!iter.second) [[unlikely]]
       {
            RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
            return;
       }
    }

    template<typename sericalize_proto, auto func, typename Self = void>
    inline coro::Task<std::pair<Errc, std::string>> ExecuteCore(std::string_view data, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        // 定义参数类型
        using param_type = std::conditional_t<
            std::is_member_function_pointer_v<func_type>,
            typename ToolBox::function_traits<func_type>::template args_tuple<1>,  // 成员函数：跳过第一个this参数
            typename ToolBox::function_traits<func_type>::args_tuple              // 普通函数：使用全部参数
        >;
        if constexpr(!std::is_void_v<param_type>){
            using First = std::tuple_element_t<0, param_type>;
            constexpr bool is_conn = requires {typename First::return_type;};
            static_assert(!is_conn, "context<T> is not allowed as parameter in coroutine function");
            bool is_ok = true;
            constexpr size_t size = std::tuple_size_v<param_type>;
            param_type args;
            if constexpr(size > 0)
            {
                is_ok = CoroRpcTools::DeserializeArg(data, args);
            }
            if (!is_ok)
            {
                
            }
        }
    }
};


}   // namespace ToolBox::CoroRpc
}   // namespace ToolBox