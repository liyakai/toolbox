#pragma once

#include <future>
#include <variant>
#include <unordered_map>
#include <functional>
#include <atomic>

#include "tools/cpp20_coroutine.h"
#include "tools/timer.h"
#include "protocol/coro_rpc_protocol.h"

namespace ToolBox {
namespace CoroRpc {

template<typename T, auto func>
concept ClientHasGenRegisterKey = requires() {
    T::template GenRegisterKey<func>();
};

// 获取全局唯一的client_id
inline uint64_t get_global_client_id() {
    static std::atomic<uint64_t> client_id{0};
    return client_id.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
struct rpc_return_type{
    using type = T;
};

template<>
struct rpc_return_type<void>{
    using type = std::monostate;
};

struct resp_body{
    int serialize_type_;
    std::string_view body_buffer_;
    std::string_view resp_attachment_buf_;
};

template<typename T>
struct async_rpc_result_value_t{
public:
    async_rpc_result_value_t() = default;
    async_rpc_result_value_t(T &&result, resp_body &&buffer)
    : result_(std::move(result)), buffer_(std::move(buffer)) {}
    async_rpc_result_value_t(T &&result)
    : result_(std::move(result)) {}

    // 允许直接访问value
    T& operator*() & { return result_; }
    const T& operator*() const & { return result_; }
    T&& operator*() && { return std::move(result_); }
    
    // 允许像指针一样使用
    T* operator->() { return &result_; }
    const T* operator->() const { return &result_; }
    
    // 隐式转换为T的引用
    operator T&() & { return result_; }
    operator const T&() const & { return result_; }
    operator T&&() && { return std::move(result_); }

    std::string_view GetRespAttachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body ReleaseBuffer() noexcept { return std::move(buffer_); }

private:
    T result_;
    resp_body buffer_;
};

template<>
struct async_rpc_result_value_t<void>{
    async_rpc_result_value_t() = default;
    resp_body buffer_;

    std::string_view GetRespAttachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body ReleaseBuffer() noexcept { return std::move(buffer_); }

};
template<typename T>
using async_rpc_result = std::variant<async_rpc_result_value_t<T>, CoroRpc::Errc>;

template<typename T>
using async_rpc_type_t = typename rpc_return_type<T>::type;

template<typename T>
using rpc_result = std::tuple<T, CoroRpc::Errc>;

template <typename rpc_protocol>
class CoroRpcClient {
private:
    using PromiseCallback = std::function<void()>;
    using SendCallback = std::function<void(std::string &&)>;
    using rpc_func_key = typename CoroRpcTools::rpc_func_key;
    struct Config {
        uint64_t client_id = get_global_client_id();
        std::chrono::milliseconds Timeout_ = std::chrono::seconds(10);
    };
    struct async_rpc_raw_result_value_type{
        resp_body buffer_;
        uint8_t errc_;
    };
    using async_rpc_raw_result = std::variant<async_rpc_raw_result_value_type, CoroRpc::Errc>;
    struct control_t
    {
        struct handler_t
        {
            ToolBox::HTIMER timer_;
            std::promise<async_rpc_raw_result> promise_;
            const PromiseCallback &callback_;
            handler_t(ToolBox::HTIMER && timer,
                        std::promise<async_rpc_raw_result> && promise,
                        PromiseCallback &&callback)
                : timer_(std::move(timer)), promise_(std::move(promise)), callback_(std::move(callback)) {}
            void operator()(resp_body &&buffer, uint8_t rpc_errc) {
                // RpcLogDebug("[rpc][client] promise set value is called, rpc_errc: %d", rpc_errc);
                ToolBox::TimerMgr->KillTimer(timer_);
                promise_.set_value(async_rpc_raw_result{async_rpc_raw_result_value_type{std::move(buffer), rpc_errc}});
                callback_();
            }
            void local_error(CoroRpc::Errc ec) {
                RpcLogError("[rpc][client] promise local_error is called, rpc_errc: %d", ec);
                ToolBox::TimerMgr->KillTimer(timer_);
                promise_.set_value(async_rpc_raw_result{ec});
                callback_();
            }
        };
        std::unordered_map<uint32_t, handler_t> response_handler_table_;
        std::atomic<uint32_t> recving_cnt_ = 0;
    };

    std::atomic<uint32_t> request_id_ = 9527;
    std::shared_ptr<control_t> control_;
    std::string_view req_attachment_;
    Config config_;
    SendCallback send_callback_ = nullptr;
public:
    struct RecvingGuard;
public:

    CoroRpcClient(uint64_t client_id = get_global_client_id())
    : control_(std::make_shared<control_t>())
     {
        config_.client_id = client_id;
    }
    ~CoroRpcClient() = default;

    // 获取配置
    const Config &get_config() const { return config_; }

    uint32_t get_client_id() const { return config_.client_id; }

    // Call RPC with default Timeout_(5s)
    template <auto func, typename... Args>
    auto Call(Args &&...args) -> ToolBox::coro::Task<async_rpc_result_value_t<std::invoke_result_t<decltype(func), Args...>>, coro::SharedLooperExecutor> 
    {
        return CallFor_<func>(std::chrono::seconds(5), std::forward<Args>(args)...);
    }

    bool SetReqAttachment(std::string_view attachment) {
        if (attachment.size() > UINT32_MAX) {
        RpcLogError("[rpc][client] attachment is too long, max length is {}",
                    UINT32_MAX);
        return false;
        }
        req_attachment_ = attachment;
        return true;
    }
    struct RecvingGuard 
    {
        RecvingGuard(control_t *ctrl):ctrl_(ctrl) { ctrl_->recving_cnt_++; }
        ~RecvingGuard() 
        {
            if (ctrl_) {
                ctrl_->recving_cnt_--;
            }
        }
        void release() { ctrl_ = nullptr; }
    private:
        control_t *ctrl_;
    };

    auto SetSendCallback(SendCallback callback)->CoroRpcClient& {
        send_callback_ = std::move(callback);
        return *this;
    }

    Errc OnRecvResp(std::string_view data) {
        return OnRecvResp_(data);
    }

private:
    // Call RPC with Timeout_
  template <auto func, typename... Args>
  auto CallFor_(auto duration, Args &&...args) 
  -> ToolBox::coro::Task<async_rpc_result_value_t<std::invoke_result_t<decltype(func), Args...>>, coro::SharedLooperExecutor> 
  {
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        auto async_result = co_await co_await SendRequestForWithAttachment<func, Args...>(
                duration, req_attachment_, std::forward<Args>(args)...);
        if (async_result.index() == 0) {
            if constexpr (std::is_void_v<return_type>) {
                co_return {};
            } else {
                co_return std::move(std::get<0>(async_result));
            }
        } else {
            co_return {};
        }
  }

  template <auto func, typename... Args>
  auto SendRequestForWithAttachment(auto time_out_duration,
                                    std::string_view request_attachment,
                                    Args &&...args)
      -> ToolBox::coro::Task<ToolBox::coro::Task<async_rpc_result<std::invoke_result_t<decltype(func), Args...>>, coro::SharedLooperExecutor>, coro::SharedLooperExecutor> 
      {
        req_attachment_ = {};   // 及时释放,防止混乱.
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        RecvingGuard guard(control_.get());
        uint32_t id;
        std::function<void()> promise_callback;
        typename ToolBox::coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack &&future_callback = [&promise_callback](std::function<void()> &&handle) {
            // RpcLogDebug("[rpc][client] future_callback is called");
            promise_callback = std::move(handle);
        };
        auto timer = ToolBox::TimerMgr->AddTimer([&](int times) {
            if(control_ == nullptr)
            {
                RpcLogError("[rpc][client] control is nullptr");
                return;
            }
            RpcLogError("[rpc][client] rpc Timeout_, id: %d", id);
            auto iter = control_->response_handler_table_.find(id);
            if(iter == control_->response_handler_table_.end())
            {
                RpcLogError("[rpc][client] response handler table not found, id: %d", id);
                return;
            }
            iter->second.local_error(CoroRpc::Errc::ERR_TIMEOUT);
            int ret = control_->response_handler_table_.erase(id);
            if(ret == 0)
            {
                RpcLogError("[rpc][client] response handler table erase failed, id: %d, table size: %zu", id, control_->response_handler_table_.size());
                return;
            }
            RpcLogDebug("[rpc][client] response handler table erase success, id: %d, table size: %zu", id, control_->response_handler_table_.size());
        }, (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(time_out_duration).count(), 1, __FILE__, __LINE__);

        auto result = co_await SendRequestForImpl_<func>(time_out_duration, id, std::move(timer), request_attachment, std::forward<Args>(args)...);
        // RpcLogDebug("[rpc][client] SendRequestForWithAttachment end, result:%d", result);
        auto& control = *control_;
        if (result == CoroRpc::Errc::SUCCESS) 
        {
            std::promise<async_rpc_raw_result> promise;
            auto future = promise.get_future();
            // bool is_empty = control.response_handler_table_.empty();
            auto &&[_, is_ok] = control.response_handler_table_.try_emplace(id, std::move(timer), std::move(promise), std::move(promise_callback));
            if (!is_ok) [[unlikely]]
            {
                RpcLogError("[rpc][client] response handler table insert failed, id: {}", id);
                co_return BuildFailedRpcResult_<return_type>(CoroRpc::Errc::ERR_SERIAL_NUMBER_CONFLICT);
            }else{
                guard.release();
                co_return DeserializeRpcResult_<return_type>(std::move(future), std::move(future_callback), std::weak_ptr<control_t>{control_});
            }
        }else {
            co_return BuildFailedRpcResult_<return_type>(std::move(result));
        }
  }

    auto Timeout_(auto duration, std::string error_msg)
    {
        co_return false;
    }

    template<typename T>
    auto HandleResponseBuffer_(int serialize_type, std::string_view buffer, uint8_t Errc) -> rpc_result<T>
    {
        // rpc_return_type<T> ret;
        // CoroRpc::Errc rpc_errc;
        if(Errc == static_cast<uint8_t>(CoroRpc::Errc::SUCCESS))
        {
            if constexpr (std::is_void_v<T>)
            {
                return rpc_result<T>{{}, CoroRpc::Errc::SUCCESS};
            } else {
                auto protocols = rpc_protocol::GetSerializeProtocolByType(serialize_type);
                if(!protocols.has_value())
                {
                    RpcLogError("[CoroRpcServer] OnRecvReq: serialize protocol not supported");
                    return rpc_result<T>{{}, CoroRpc::Errc::ERR_PROTOCOL_NOT_SUPPORTED};
                }
                
                T value;
                return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> rpc_result<T> {
                    bool is_ok = serialize_protocol::Deserialize(value, buffer);
                    if(!is_ok) [[unlikely]]
                    {
                        RpcLogError("[rpc][client] HandleResponseBuffer_ deserialize failed");
                        return rpc_result<T>{{}, CoroRpc::Errc::ERR_DESERIALIZE_FAILED};
                    }else {
                        return rpc_result<T>{std::move(value), CoroRpc::Errc::SUCCESS};
                    }
                }, protocols.value());
                

            }
        }else {
            return rpc_result<T>{{}, CoroRpc::Errc(Errc)};
        }
        
    }

    template <typename T>
    auto DeserializeRpcResult_(std::future<async_rpc_raw_result> &&future, coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack &&future_callback, std::weak_ptr<control_t> &&ctrl)
        -> ToolBox::coro::Task<async_rpc_result<T>, coro::SharedLooperExecutor> 
    {
        auto result = co_await coro::FutureAwaiter<async_rpc_raw_result>(std::move(future)).with_future_callback(std::move(future_callback));
        if(result.index() == 1) [[unlikely]]
        {
            auto &ret = std::get<1>(result);
            if(ret == CoroRpc::Errc::ERR_TIMEOUT
                || ret == CoroRpc::Errc::ERR_OPERATION_CANCELED)
            {
                RpcLogError("[rpc][client] DeserializeRpcResult_ Timeout_ or canceled, rpc_errc: %d", ret);
                co_return CoroRpc::Errc::ERR_TIMEOUT;
            } else {
                RpcLogError("[rpc][client] DeserializeRpcResult_ error, rpc_errc: %d", ret);
                co_return CoroRpc::Errc(ret);
            }
        }
        auto &ret = std::get<0>(result);
        auto resp_result = HandleResponseBuffer_<T>(ret.buffer_.serialize_type_, ret.buffer_.body_buffer_, ret.errc_);
        if(std::get<1>(resp_result) == CoroRpc::Errc::SUCCESS)
        {
            if constexpr (std::is_void_v<T>)
            {
                co_return {};
            } else {
                co_return async_rpc_result<T>{async_rpc_result_value_t<T>{std::move(std::get<0>(resp_result)), std::move(ret.buffer_)}};
            }
        }
        else {
            RpcLogError("[rpc][client] DeserializeRpcResult_ error, rpc_errc: %d", std::get<1>(resp_result));
            co_return async_rpc_result<T>{async_rpc_result_value_t<T>{std::move(std::get<0>(resp_result))}};
        }
    }

    template <typename T>
    auto BuildFailedRpcResult_(CoroRpc::Errc Errc)
        -> ToolBox::coro::Task<async_rpc_result<T>, coro::SharedLooperExecutor> 
    {
        co_return async_rpc_result<T>{Errc};
    }

    template <auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::SharedLooperExecutor>
    SendRequestForImpl_(auto duration, uint32_t &id, ToolBox::HTIMER &&timer, std::string_view attachment, Args &&...args) 
    {
        // 检查定时器
        int32_t time_left = ToolBox::TimerMgr->GetTimeLeft(timer);
        if(time_left <= 0)
        {
            co_return CoroRpc::Errc::ERR_TIMEOUT;
        }
        co_return co_await SendImpl_<func>(id, attachment, std::forward<Args>(args)...);
    }

    template <auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::SharedLooperExecutor>
    SendImpl_(uint32_t &id, std::string_view attachment, Args &&...args)
    {
        std::string send_buffer;
        auto errc = PrepareSendBuffer_<func>(id, attachment.size(), send_buffer, std::forward<Args>(args)...);
        if (errc != CoroRpc::Errc::SUCCESS) {
            RpcLogError("[rpc][client] SendImpl_ PrepareSendBuffer_ failed, rpc_errc: %d", errc);
            co_return errc;
        }
        std::pair<std::error_code, std::size_t> send_result;
        if (!attachment.empty()) 
        {
            // Append attachment data to send_buffer
            std::memcpy(send_buffer.data() + send_buffer.size() - attachment.size(), attachment.data(), attachment.size());
        }
        // RpcLogDebug("[rpc][client] SendImpl_ send_buffer size: %zu, attachment_size: %zu, send_buffer: %s", send_buffer.size(), attachment.size(), send_buffer.c_str());
        if(!send_callback_)
        {
            RpcLogError("[rpc][client] SendImpl_: send_callback_ not set");
            co_return CoroRpc::Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        send_callback_(std::move(send_buffer));
        // send buffer
        co_return CoroRpc::Errc::SUCCESS;
    }

    template<auto func, typename... Args>
    auto PrepareSendBuffer_(uint32_t &id, std::size_t attachment_size, std::string & buffer, Args &&...args) -> CoroRpc::Errc
    {
        // 编译期计算唯一key（无运行时开销）
        static constexpr rpc_func_key key = [](){
            if constexpr(ClientHasGenRegisterKey<rpc_protocol, func>) {
                return rpc_protocol::template GenRegisterKey<func>();
            } else {
                return CoroRpcTools::AutoGenRegisterKey<func>();
            }
        }();

        auto serialize_proto = rpc_protocol::GetSerializeProtocolByType(rpc_protocol::SERIALIZE_TYPE); 
        if(!serialize_proto.has_value())
        {
            RpcLogError("[rpc][client] PrepareSendBuffer_: serialize protocol not supported");
            return CoroRpc::Errc::ERR_PROTOCOL_NOT_SUPPORTED;
        }

        return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> CoroRpc::Errc {
            uint32_t body_length = serialize_protocol::SerializeSize(std::forward<Args>(args)...);
            if(body_length > UINT32_MAX) [[unlikely]]
            {
                RpcLogError("[rpc][client] attachment is too long, max length is %u", UINT32_MAX);
                return CoroRpc::Errc::ERR_MESSAGE_TOO_LARGE;
            }
            buffer.resize(rpc_protocol::REQ_HEAD_LEN + body_length + attachment_size);
            // 使用placement new初始化ReqHeader（按照结构体字段声明顺序）
            new (buffer.data()) rpc_protocol::ReqHeader{
                .magic = rpc_protocol::MAGIC_NUMBER,
                .version = rpc_protocol::VERSION_NUMBER,
                .serialize_type = rpc_protocol::SERIALIZE_TYPE,
                .msg_type = 0,
                .seq_num = ++request_id_,
                .func_id = key,
                .length = body_length,
                .attach_length = static_cast<uint32_t>(attachment_size)
            };
            // 保存序列号用于后续使用
            id = request_id_;
            return serialize_protocol::SerializeToBuffer(
                buffer.data() + rpc_protocol::REQ_HEAD_LEN,
                buffer.size() - rpc_protocol::REQ_HEAD_LEN - attachment_size,
                std::forward<Args>(args)...);
        }, serialize_proto.value());
    }

    Errc OnRecvResp_(std::string_view data) {
        typename rpc_protocol::RespHeader header;
        auto err = rpc_protocol::ReadHeader(data, header);
        if(err != Errc::SUCCESS)
        {
            RpcLogError("[CoroRpcServer] OnRecvReq: read header failed, err:%d", err);
            return err;
        }
        std::string_view body;
        std::string_view attachment;
        err = rpc_protocol::ReadPayLoad(header, data.substr(rpc_protocol::RESP_HEAD_LEN), body, attachment);
        if(err != CoroRpc::Errc::SUCCESS)
        {
            RpcLogError("[rpc][client] OnRecvResp_ read payload failed, err:%d", err);
            return err;
        }
        if(auto iter = control_->response_handler_table_.find(header.seq_num); iter != control_->response_handler_table_.end())
        {
            iter->second(resp_body(header.serialize_type, std::move(body), std::move(attachment)), header.err_code);
        }
        return CoroRpc::Errc::SUCCESS;
    }



private:
    


};  // class CoroRpcClient

}; // namespace CoroRpc
}; // namespace ToolBox

