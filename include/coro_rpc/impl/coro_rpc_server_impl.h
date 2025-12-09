#pragma once

#include <mutex>
#include <chrono>

#include "coro_rpc_def_interenal.h"
#include "protocol/coro_rpc_protocol.h"
#include "protocol/serialize_adapter.h"
#include "stream_rpc.h"
#include "tools/cpp20_coroutine.h"
#include "tools/function_traits.h"

namespace ToolBox {
namespace CoroRpc {

template<typename T, auto func>
concept ServerHasGenRegisterKey = requires() {
    T::template GenRegisterKey<func>();
};

// 检测是否是StreamGenerator类型
template<typename T>
struct IsStreamGenerator : std::false_type {};

template<typename T>
struct IsStreamGenerator<StreamGenerator<T>> : std::true_type {};

template<typename T>
inline constexpr bool kIsStreamGeneratorV = IsStreamGenerator<T>::value;

template <typename rpc_protocol, template<typename...> typename map_t>
class CoroRpcServer {
private:
    using handler_t = std::function<std::pair<Errc, std::string>(std::string_view, std::string_view, typename rpc_protocol::supported_serialize_protocols)>;
    using core_handler_t = std::function<ToolBox::coro::Task<std::pair<Errc, std::string>>(std::string_view, std::string_view, typename rpc_protocol::supported_serialize_protocols protocols)>;
    // 流式处理器类型：返回StreamGenerator<std::string>（已序列化的数据）
    using stream_handler_t = std::function<StreamGenerator<std::string>(std::string_view, std::string_view, typename rpc_protocol::supported_serialize_protocols protocols)>;
    using rpc_func_key = typename CoroRpcTools::rpc_func_key_t;
    using SendCallback = std::function<void(uint64_t opaque, std::string &&)>;

    SendCallback send_callback_ = nullptr;
    map_t<rpc_func_key, handler_t> rpc_server_handler_map_;
    map_t<rpc_func_key, core_handler_t> rpc_server_core_handler_map_;
    // 流式处理器映射
    map_t<rpc_func_key, stream_handler_t> stream_handler_map_;
    map_t<rpc_func_key, std::string> func_key2name_map_;
    map_t<std::string, rpc_func_key> name2func_key_map_;
    std::string_view req_attachment_;
    map_t<rpc_func_key, std::function<std::string_view()>> resp_attachment_func_map_;
    
    // 流状态管理：stream_id -> 流状态
    struct StreamState {
        bool paused = false;      // 是否暂停（背压）
        bool cancelled = false;   // 是否已取消
        uint64_t opaque = 0;      // 连接标识
        rpc_func_key key = 0;    // 函数key
    };
    std::mutex stream_state_mutex_;  // 保护流状态表的互斥锁
    map_t<uint32_t, StreamState> stream_state_map_;  // stream_id -> StreamState
public:
    CoroRpcServer() = default;
    ~CoroRpcServer() = default;

    /*
    * @brief Register RPC service functions(member function)
    */
    template <auto first, auto... functions>
    void RegisterService(typename ToolBox::FunctionTraits<decltype(first)>::class_type *self) {
        RegisterOneHandler<first>(self);
        (RegisterOneHandler<functions>(self), ...);
    }

    template <auto first>
    void RegisterService(typename ToolBox::FunctionTraits<decltype(first)>::class_type *self, const auto &key) {
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

    const std::string& GetFunctionKeyName(const rpc_func_key &key) const {
        static std::string empty_str;
        auto it = func_key2name_map_.find(key);
        if (it == func_key2name_map_.end()) {
            return empty_str;
        }
        return it->second;
    }

    Errc OnRecvReq(uint64_t opaque, std::string_view data) {
        return OnRecvReq_(opaque, data);
    }
    Errc SetSendCallback(SendCallback &&callback) {
        send_callback_ = std::move(callback);
        return Errc::SUCCESS;
    }
    template <auto func>
    void SetRespAttachmentFunc(std::function<std::string_view()> &&resp_attachment_func) {
        static const rpc_func_key key = [] {
            if constexpr (ServerHasGenRegisterKey<rpc_protocol, func>) {
                return rpc_protocol::template GenRegisterKey<func>();
            } else {
                return CoroRpcTools::AutoGenRegisterKey<func>();
            }
        }();

        // 每次调用时，用 key 在当前 this 的 map 上访问
        resp_attachment_func_map_[key] = std::move(resp_attachment_func);
    }
    
    std::string_view GetReqAttachment() const noexcept { return req_attachment_; }

    std::vector<rpc_func_key> GetAllServiceHandlerKeys()
    {
        std::vector<rpc_func_key> keys;
        keys.reserve(rpc_server_handler_map_.size() + rpc_server_core_handler_map_.size());
        for(auto &iter : rpc_server_handler_map_)
        {
            keys.push_back(iter.first);
        }
        for(auto &iter : rpc_server_core_handler_map_)
        {
            keys.push_back(iter.first);
        }
        for(auto &iter : stream_handler_map_)
        {
            keys.push_back(iter.first);
        }

        return keys;
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
        if constexpr(ServerHasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }

        RpcLogDebug("[rpc][server] RegisterOneHandler.class: key: %u, func: %.*s", key, static_cast<int>(ToolBox::GetFuncName<func>().size()), ToolBox::GetFuncName<func>().data());

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
        using ReturnType = typename ToolBox::FunctionTraits<decltype(func)>::return_type;
        constexpr auto name = ToolBox::GetFuncNameWithClass<func>();

        RpcLogInfo("[CoroRpcServer] RegistOneHandlerImpl: key: %u, func: %.*s",
                   key, static_cast<int>(name.size()), name.data());

        // 检查是否是流式RPC（返回StreamGenerator）
        if constexpr (kIsStreamGeneratorV<ReturnType>)
        {
            // 流式RPC处理
            using StreamValueType = typename ReturnType::value_type;
            auto iter = stream_handler_map_.emplace(key, [&, self](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> StreamGenerator<std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> StreamGenerator<std::string> {
                    if constexpr (std::is_void_v<typename ToolBox::FunctionTraits<decltype(func)>::class_type>) {
                        return ExecuteStream_<serialize_protocol, func, StreamValueType>(data, attachment, nullptr);
                    } else {
                        return ExecuteStream_<serialize_protocol, func, StreamValueType>(data, attachment, self);
                    }
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: stream handler key already exists");
                return;
            }
            RpcLogInfo("[CoroRpcServer] Registered stream handler for key: %u", key);
        }
        else if constexpr(ToolBox::is_specialization_v<ReturnType, ToolBox::coro::Task>)
        {
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                    co_return co_await ExecuteCoro_<serialize_protocol, func>(data, attachment, self);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    // Execute_ 内部已经有兼容性检查，这里可以移除冗余检查
                    return Execute_<serialize_protocol, func>(data, attachment, self);
                }, protocols);
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
        if constexpr(ServerHasGenRegisterKey<rpc_protocol, func>)
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
            auto iter = rpc_server_core_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> ToolBox::coro::Task<std::pair<Errc, std::string>> {
                    co_return co_await ExecuteCoro_<serialize_protocol, func>(data, attachment);
                }, protocols);
            });
            if (!iter.second) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] RegistOneHandlerImpl: key already exists");
                return;
            }
        } else {
            auto iter = rpc_server_handler_map_.emplace(key, [&](std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols) -> std::pair<Errc, std::string> {
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> std::pair<Errc, std::string> {
                    return Execute_<serialize_protocol, func>(data, attachment);
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

    ToolBox::coro::Task<std::pair<Errc, std::string>> HandleCoro(auto handler, std::string_view data, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols, const typename CoroRpcTools::rpc_func_key_t &key)
    {
        using namespace std::string_literals;
        if(handler) [[likely]]
        {
            try {
                co_return co_await handler(data, attachment, protocols);
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

    std::pair<Errc, std::string> Handle(auto handler, std::string_view payload, std::string_view attachment, typename rpc_protocol::supported_serialize_protocols protocols)
    {
        using namespace std::string_literals;
        if(handler) [[likely]]
        {
            try {
                return handler(payload, attachment, protocols);
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

    // 统一的函数调用辅助：处理 Self 和参数的组合
    // 有参数版本
    template<auto func, typename Self, typename ArgsTuple>
    requires (!std::is_void_v<ArgsTuple>)
    static auto invoke_with_params(Self* self, ArgsTuple&& args) {
        if constexpr (std::is_void_v<Self>) {
            return std::apply(func, std::forward<ArgsTuple>(args));
        } else {
            return std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::forward<ArgsTuple>(args)));
        }
    }
    
    // 无参数版本
    template<auto func, typename Self>
    static auto invoke_with_params(Self* self) {
        if constexpr (std::is_void_v<Self>) {
            return func();
        } else {
            return std::apply(func, std::forward_as_tuple(*self));
        }
    }
    
    // 统一的函数调用辅助（协程版本）- 有参数版本
    template<auto func, typename Self, typename ArgsTuple>
    requires (!std::is_void_v<ArgsTuple>)
    static auto invoke_with_params_coro(Self* self, ArgsTuple&& args) {
        if constexpr (std::is_void_v<Self>) {
            return std::apply(func, std::forward<ArgsTuple>(args));
        } else {
            return std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::forward<ArgsTuple>(args)));
        }
    }
    
    // 无参数版本
    template<auto func, typename Self>
    static auto invoke_with_params_coro(Self* self) {
        if constexpr (std::is_void_v<Self>) {
            return func();
        } else {
            return std::apply(func, std::forward_as_tuple(*self));
        }
    }
    
    // 统一的执行逻辑：反序列化 -> 调用函数 -> 序列化返回值
    template<typename serialize_proto, auto func, typename Self = void>
    inline std::pair<Errc, std::string> Execute_(std::string_view data, std::string_view attachment, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using ReturnType = typename traits_type::return_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        req_attachment_ = attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] Execute_: deserialize arguments failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] Execute_: Protocol compatibility check failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                invoke_with_params<func>(self, std::move(args));
                return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = invoke_with_params<func>(self, std::move(args));
                return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        } else {
            // 无参数函数
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] Execute_: Protocol compatibility check failed");
                return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                invoke_with_params<func>(self);
                return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = invoke_with_params<func>(self);
                return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        }
    }

    template<typename serialize_proto, auto func, typename Self = void>
    inline coro::Task<std::pair<Errc, std::string>> ExecuteCoro_(std::string_view data, std::string_view attachment, Self *self = nullptr)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using ReturnType = typename traits_type::return_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        req_attachment_ = attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[rpc] Deserialize failed. data size:%zu, attachment size:%zu", data.size(), attachment.size());
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "deserialize arguments failed"s);
            }
            
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] ExecuteCoro_: Protocol compatibility check failed");
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                co_await invoke_with_params_coro<func>(self, std::move(args));
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = co_await invoke_with_params_coro<func>(self, std::move(args));
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        } else {
            // 无参数函数
            // 2. 防御性兼容性检查
            if constexpr (!std::is_void_v<ReturnType> && !adapter::is_compatible()) {
                RpcLogError("[CoroRpcServer] ExecuteCoro_: Protocol compatibility check failed");
                co_return std::make_pair(Errc::ERR_INVALID_ARGUMENTS, "protocol mismatch"s);
            }
            
            // 3. 调用函数并序列化返回值
            if constexpr (std::is_void_v<ReturnType>)
            {
                co_await invoke_with_params_coro<func>(self);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize());
            } else {
                auto result = co_await invoke_with_params_coro<func>(self);
                co_return std::make_pair(Errc::SUCCESS, adapter::serialize(result));
            }
        }
    }   // ExecuteCoro_
    
    // 执行流式RPC（成员函数版本）
    template<typename serialize_proto, auto func, typename StreamValueType, typename Self>
    requires (!std::is_void_v<Self>)
    inline StreamGenerator<std::string> ExecuteStream_(std::string_view data, std::string_view attachment, Self *self)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        req_attachment_ = attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] ExecuteStream_: deserialize arguments failed");
                co_yield "";  // 返回错误标记
                co_return;
            }
            
            // 2. 调用函数获取StreamGenerator
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::tuple_cat(std::forward_as_tuple(*self), std::move(args)));
            
            // 3. 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                std::string serialized = adapter::serialize(value);
                co_yield std::move(serialized);
            }
        } else {
            // 无参数函数
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::forward_as_tuple(*self));
            
            // 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                std::string serialized = adapter::serialize(value);
                co_yield std::move(serialized);
            }
        }
    }
    
    // 执行流式RPC（非成员函数版本）
    template<typename serialize_proto, auto func, typename StreamValueType>
    inline StreamGenerator<std::string> ExecuteStream_(std::string_view data, std::string_view attachment, std::nullptr_t)
    {
        using namespace std::string_literals;
        using func_type = decltype(func);
        using traits_type = ToolBox::FunctionTraits<func_type>;
        using param_type = typename traits_type::parameters_type;
        using adapter = SerializeAdapter<func, serialize_proto>;

        req_attachment_ = attachment;
        
        // 1. 反序列化参数（如果有）
        if constexpr(!std::is_void_v<param_type> && std::tuple_size_v<param_type> > 0)
        {
            param_type args;
            if (!serialize_proto::Deserialize(args, data)) [[unlikely]]
            {
                RpcLogError("[CoroRpcServer] ExecuteStream_: deserialize arguments failed");
                co_yield "";  // 返回错误标记
                co_return;
            }
            
            // 2. 调用函数获取StreamGenerator
            StreamGenerator<StreamValueType> stream_gen = std::apply(func, std::move(args));
            
            // 3. 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                std::string serialized = adapter::serialize(value);
                co_yield std::move(serialized);
            }
        } else {
            // 无参数函数
            StreamGenerator<StreamValueType> stream_gen = func();
            
            // 遍历流式数据并序列化
            while (stream_gen.Next()) {
                auto& value = stream_gen.value();
                std::string serialized = adapter::serialize(value);
                co_yield std::move(serialized);
            }
        }
    }
    
    // 处理流式请求：持续发送流式数据
    void HandleStreamRequest_(uint64_t opaque, rpc_func_key key, const typename rpc_protocol::ReqHeader& req_header, StreamGenerator<std::string>&& generator)
    {
        uint32_t stream_id = req_header.seq_num;
        
        // 注册流状态
        {
            std::lock_guard<std::mutex> lock(stream_state_mutex_);
            stream_state_map_[stream_id] = StreamState{false, false, opaque, key};
        }
        
        // 在后台协程中处理流式响应
        auto stream_task = [this, opaque, key, req_header, stream_id, gen = std::move(generator)]() mutable -> ToolBox::coro::Task<std::monostate> {
            bool is_first = true;
            while (gen.Next()) {
                // 检查流是否已取消
                {
                    std::lock_guard<std::mutex> lock(stream_state_mutex_);
                    auto iter = stream_state_map_.find(stream_id);
                    if (iter != stream_state_map_.end() && iter->second.cancelled) {
                        RpcLogInfo("[CoroRpcServer] HandleStreamRequest_: stream cancelled, stream_id: %u", stream_id);
                        break;
                    }
                }
                
                // 检查是否需要暂停（背压控制）
                bool need_pause = false;
                {
                    std::lock_guard<std::mutex> lock(stream_state_mutex_);
                    auto iter = stream_state_map_.find(stream_id);
                    if (iter != stream_state_map_.end()) {
                        need_pause = iter->second.paused;
                        // 如果已取消，直接退出
                        if (iter->second.cancelled) {
                            RpcLogInfo("[CoroRpcServer] HandleStreamRequest_: stream cancelled, stream_id: %u", stream_id);
                            break;
                        }
                    }
                }
                
                // 如果暂停，跳过这次发送，让协程让出执行权
                if (need_pause) {
                    // 使用简单的延迟：让协程让出执行权，等待下一次调度
                    // 通过 continue 跳过这次数据发送，协程会自然让出执行权
                    continue;
                }
                
                std::string data = std::move(gen.value());
                uint8_t msg_type;
                if (is_first) {
                    msg_type = static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart);
                    is_first = false;
                } else {
                    msg_type = static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamData);
                }
                
                // 发送流式数据包
                Errc err = SendStreamResponse_(opaque, key, req_header, std::move(data), msg_type);
                if (err != Errc::SUCCESS) {
                    RpcLogError("[CoroRpcServer] HandleStreamRequest_: send stream response failed, err: %d", err);
                    break;
                }
            }
            
            // 检查是否已取消，然后清理流状态并发送结束标记
            bool was_cancelled = false;
            {
                std::lock_guard<std::mutex> lock(stream_state_mutex_);
                auto iter = stream_state_map_.find(stream_id);
                if (iter != stream_state_map_.end()) {
                    was_cancelled = iter->second.cancelled;
                    stream_state_map_.erase(iter);
                }
            }
            
            // 发送流结束标记（如果未取消）
            if (!was_cancelled) {
                SendStreamResponse_(opaque, key, req_header, "", static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamEnd));
            }
            
            co_return std::monostate{};
        }();
        stream_task.then([](auto) {
            // 流式响应处理完成
        });
    }
    
    // 发送流式响应
    Errc SendStreamResponse_(uint64_t opaque, rpc_func_key key, const typename rpc_protocol::ReqHeader& req_header, std::string&& data, uint8_t msg_type)
    {
        std::string resp_buf;
        resp_buf.resize(rpc_protocol::RESP_HEAD_LEN + data.size());
        
        bool prepare_ret = rpc_protocol::PrepareResponseHeader(resp_buf, data, req_header, 0, msg_type, Errc::SUCCESS);
        if (!prepare_ret) {
            RpcLogError("[CoroRpcServer] SendStreamResponse_: prepare response header failed");
            return Errc::ERR_SERVER_PREPARE_HEADER_FAILED;
        }
        
        std::memcpy(resp_buf.data() + rpc_protocol::RESP_HEAD_LEN, data.data(), data.size());
        
        if (!send_callback_) {
            RpcLogError("[CoroRpcServer] SendStreamResponse_: send callback not set");
            return Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        
        send_callback_(opaque, std::move(resp_buf));
        return Errc::SUCCESS;
    }
    
    Errc OnRecvReq_(uint64_t opaque, std::string_view data)
    {
        typename rpc_protocol::ReqHeader header;
        auto err = rpc_protocol::ReadHeader(data, header);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read header failed, err:%d", err);
            return err;
        }

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
            RpcLogError("[CoroRpcServer] OnRecvReq: read payload failed, err: %d", err);
            return err;
        }

        Errc resp_err = Errc::SUCCESS;
        std::string rpc_result;
        auto key = rpc_protocol::GetRpcFuncKey(header);
        uint32_t stream_id = header.seq_num;
        
        // 检查是否是流式RPC控制消息（取消、暂停、恢复）
        if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamCancel)) {
            // 处理流取消请求
            std::lock_guard<std::mutex> lock(stream_state_mutex_);
            auto iter = stream_state_map_.find(stream_id);
            if (iter != stream_state_map_.end()) {
                iter->second.cancelled = true;
                RpcLogInfo("[CoroRpcServer] OnRecvReq: stream cancelled by client, stream_id: %u", stream_id);
            }
            return Errc::SUCCESS;
        } else if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamPause)) {
            // 处理流暂停请求（背压控制）
            std::lock_guard<std::mutex> lock(stream_state_mutex_);
            auto iter = stream_state_map_.find(stream_id);
            if (iter != stream_state_map_.end()) {
                iter->second.paused = true;
                RpcLogInfo("[CoroRpcServer] OnRecvReq: stream paused (backpressure), stream_id: %u", stream_id);
            }
            return Errc::SUCCESS;
        } else if (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamResume)) {
            // 处理流恢复请求（背压控制）
            std::lock_guard<std::mutex> lock(stream_state_mutex_);
            auto iter = stream_state_map_.find(stream_id);
            if (iter != stream_state_map_.end()) {
                iter->second.paused = false;
                RpcLogInfo("[CoroRpcServer] OnRecvReq: stream resumed (backpressure), stream_id: %u", stream_id);
            }
            return Errc::SUCCESS;
        }
        
        // 检查是否是流式RPC请求
        bool is_stream_request = (header.msg_type == static_cast<uint8_t>(rpc_protocol::MsgType::kMsgTypeStreamStart));
        
        if (is_stream_request) {
            // 处理流式RPC请求
            auto stream_iter = stream_handler_map_.find(key);
            if (stream_iter != stream_handler_map_.end()) {
                // 执行流式RPC，获取StreamGenerator
                auto stream_gen = stream_iter->second(payload, attachment, serialize_protocol.value());
                
                // 启动流式响应处理（在后台协程中）
                HandleStreamRequest_(opaque, key, header, std::move(stream_gen));
                return Errc::SUCCESS;
            } else {
                RpcLogError("[CoroRpcServer] OnRecvReq: stream handler not found for key: %u", key);
                return Errc::ERR_FUNC_NOT_REGISTERED;
            }
        }
        
        // 普通RPC处理
        auto handler = GetHandler(key);
        if(handler == nullptr)
        {
            auto core_handler = GetCoreHandler(key);
            auto handler_ret = HandleCoro(core_handler, payload, attachment, serialize_protocol.value(), key);
            handler_ret.then([this, opaque, key, header](std::pair<Errc, std::string> &&result) {
                if(result.first != Errc::SUCCESS)
                {
                    RpcLogError("[CoroRpcServer] OnRecvReq: handler error, err:%d", result.first);
                    return result.first;
                }
                ResponseMsgAttachment_(opaque, key, result.first, result.second, header);
                return Errc::SUCCESS;
            });
            return Errc::SUCCESS;
        }else {
            std::tie(resp_err, rpc_result) = Handle(handler, payload, attachment, serialize_protocol.value());
            if(resp_err != Errc::SUCCESS)
            {
                RpcLogError("[CoroRpcServer] OnRecvReq: handler error, err:%d", resp_err);
                return resp_err;
            }
            return ResponseMsgAttachment_(opaque, key, resp_err, rpc_result, header);
        }
    }
    Errc ResponseMsgAttachment_(uint64_t opaque, CoroRpcTools::rpc_func_key key, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_header)
    {
        Errc ret = Errc::SUCCESS;
        auto resp_attachment_func_iter = resp_attachment_func_map_.find(key);
        if(resp_attachment_func_iter != resp_attachment_func_map_.end())
        {
            ret = DirectResponseMsg(opaque, resp_err, rpc_result, req_header, std::move(resp_attachment_func_iter->second));
            // resp_attachment_func_map_.erase(resp_attachment_func_iter);
        } else {
            ret = DirectResponseMsg(opaque, resp_err, rpc_result, req_header);
        }
        if(ret != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: prepare response header failed");
            return ret;
        }
        return Errc::SUCCESS;
    }
    Errc DirectResponseMsg(uint64_t opaque, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head) {
        return DirectResponseMsg(opaque, resp_err, std::move(rpc_result), req_head, 
                               [](){ return std::string_view{}; });
    }
    Errc DirectResponseMsg(uint64_t opaque, Errc resp_err, const std::string & rpc_result, 
        const typename rpc_protocol::ReqHeader &req_head,
        std::function<std::string_view()> &&attachment_func)
    {
        std::string resp_error_msg;
        if(resp_err != Errc::SUCCESS)
        {
            resp_error_msg = rpc_result;
            RpcLogWarn("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", resp_err);
        }
        std::string_view attachment = attachment_func();
        std::string resp_buf_bytes;
        resp_buf_bytes.resize(rpc_protocol::RESP_HEAD_LEN + rpc_result.size() + attachment.length());
        // RpcLogDebug("[rpc][server] DirectResPonseMsg.resp_buf_bytes: resp_buf_bytes size: %zu, rpc_result: %zu, attachment_size: %zu", resp_buf_bytes.size(), rpc_result.size(), attachment.length());
        bool prepare_ret = rpc_protocol::PrepareResponseHeader(resp_buf_bytes, rpc_result, req_head, attachment.length(), resp_err, resp_error_msg);
        if(!prepare_ret)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: prepare response header failed");
            return Errc::ERR_SERVER_PREPARE_HEADER_FAILED;
        }
        auto response_ret = Response(opaque, std::move(resp_buf_bytes), rpc_result, std::move(attachment));
        Errc err_code = response_ret.get_result();
        if(err_code != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] DirectResPonseMsg: response error, err:%d", err_code);
            return err_code;
        }
        return Errc::SUCCESS;
    }
    ToolBox::coro::Task<Errc> Response(uint64_t opaque, std::string &&resp_buf_bytes, const std::string &rpc_result, std::string_view &&attachment)
    {
        if(!send_callback_)
        {
            RpcLogError("[CoroRpcServer] Response: send callback not set");
            co_return Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN, rpc_result.data(), rpc_result.size());
        if(attachment.length() > 0)
        {
            std::memcpy(resp_buf_bytes.data() + rpc_protocol::RESP_HEAD_LEN + rpc_result.size(), attachment.data(), attachment.size());
        }
        send_callback_(opaque, std::move(resp_buf_bytes));
        co_return Errc::SUCCESS;
    }
};  // class CoroRpcServer

}   // namespace ToolBox::CoroRpc
}   // namespace ToolBox