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
concept HasGenRegisterKey = requires() {
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
    std::string body_buffer_;
    std::string resp_attachment_buf_;
};

template<typename T>
struct async_rpc_result_value_t{
public:
    async_rpc_result_value_t() = default;
    async_rpc_result_value_t(T &&result, resp_body &&buffer)
    : result_(std::move(result)), buffer_(std::move(buffer)) {}
    async_rpc_result_value_t(T &&result)
    : result_(std::move(result)) {}
    T &result() noexcept { return result_; }
    const T &result() const noexcept { return result_; }
    std::string_view get_attachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body release_buffer() noexcept { return std::move(buffer_); }

private:
    T result_;
    resp_body buffer_;
};

template<>
struct async_rpc_result_value_t<void>{
    async_rpc_result_value_t() = default;
    resp_body buffer_;

    std::string_view get_attachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body release_buffer() noexcept { return std::move(buffer_); }

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
    struct handler_t;
    struct Config {
        uint64_t client_id = get_global_client_id();
        std::chrono::milliseconds Timeout_ = std::chrono::seconds(10);
    };
    struct control_t
    {
      bool is_timeout_ = false;
      std::unordered_map<uint32_t, handler_t> response_handler_table_;
      control_t(bool is_timeout) : is_timeout_(is_timeout) {}
      std::atomic<uint32_t> recving_cnt_ = 0;
      resp_body resp_buffer_;

    };
    struct async_rpc_raw_result_value_type{
        resp_body buffer_;
        uint8_t errc_;
    };
    using async_rpc_raw_result = std::variant<async_rpc_raw_result_value_type, CoroRpc::Errc>;
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
            RpcLogDebug("[rpc][client] promise set value is called, rpc_errc: %d", rpc_errc);
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

    uint64_t client_id_;
    bool should_reset_ = false;
    std::atomic<uint32_t> request_id_ = 9527;
    ToolBox::HTIMER timer_;
    std::shared_ptr<control_t> control_;
    std::string_view req_attachment_;
    Config config_;
    SendCallback send_callback_ = nullptr;
public:
    struct RecvingGuard;
public:

    CoroRpcClient(uint64_t client_id = get_global_client_id())
    : control_(std::make_shared<control_t>(false))
     {
        config_.client_id = client_id;
    }
    ~CoroRpcClient() = default;

    // 获取配置
    const Config &get_config() const { return config_; }

    uint32_t get_client_id() const { return config_.client_id; }

    // Call RPC with default Timeout_(5s)
    template <auto func, typename... Args>
    auto Call(Args &&...args) -> ToolBox::coro::Task<std::invoke_result_t<decltype(func), Args...>, coro::NewThreadExecutor> {
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
    std::string_view GetRespAttachment() const noexcept { return control_->resp_buffer_.resp_attachment_buf_; }

    
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
        -> ToolBox::coro::Task<std::invoke_result_t<decltype(func), Args...>, coro::NewThreadExecutor> {
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        auto async_result = co_await co_await SendRequestForWithAttachment<func, Args...>(
                duration, req_attachment_, std::forward<Args>(args)...);
        fprintf(stderr, "[rpc][client] CallFor_ inner_task is created\n");
        // auto async_result = co_await inner_task;
        fprintf(stderr, "[rpc][client] CallFor_ async_result index: %zu\n", async_result.index());
        req_attachment_ = {};
        if (async_result.index() == 0) {
            if constexpr (std::is_void_v<return_type>) {
                co_return {};
            } else {
                co_return std::move(std::get<0>(async_result).result());
            }
        } else {
            co_return {};
        }
    }

    
    template <auto func, typename... Args>
    auto SendRequestForWithAttachment(
        auto time_out_duration, std::string_view request_attachment,
        Args &&...args) -> ToolBox::coro::Task<ToolBox::coro::Task<async_rpc_result<std::invoke_result_t<decltype(func), Args...>>, coro::NewThreadExecutor>, coro::NewThreadExecutor>
    {
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        RecvingGuard guard(control_.get());
        uint32_t id;
        std::function<void()> promise_callback;
        typename ToolBox::coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack &&future_callback = [&promise_callback](std::function<void()> &&handle) {
            RpcLogDebug("[rpc][client] future_callback is called");
            promise_callback = std::move(handle);
        };
        auto timer = ToolBox::TimerMgr->AddTimer([&](int times) {
            if(control_ == nullptr)
            {
                RpcLogError("[rpc][client] control is nullptr");
                return;
            }
            control_->is_timeout_ = true;
            RpcLogError("[rpc][client] rpc Timeout_, id: %d", id);
            auto iter = control_->response_handler_table_.find(id);
            if(iter == control_->response_handler_table_.end())
            {
                RpcLogError("[rpc][client] response handler table not found, id: %d", id);
                return;
            }
            fprintf(stderr, "[rpc][client] response handler table erase before local_error, id: %d, table size: %zu\n", id, control_->response_handler_table_.size());
            iter->second.local_error(CoroRpc::Errc::ERR_TIMEOUT);
            fprintf(stderr, "[rpc][client] response handler table erase after local_error, id: %d, table size: %zu\n", id, control_->response_handler_table_.size());
            int ret = control_->response_handler_table_.erase(id);
            if(ret == 0)
            {
                RpcLogError("[rpc][client] response handler table erase failed, id: %d, table size: %zu", id, control_->response_handler_table_.size());
                return;
            }
            RpcLogDebug("[rpc][client] response handler table erase success, id: %d, table size: %zu", id, control_->response_handler_table_.size());
        }, (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(time_out_duration).count(), 1, __FILE__, __LINE__);

        auto result = co_await SendRequestForImpl_<func>(time_out_duration, id, std::move(timer), request_attachment, std::forward<Args>(args)...);
        RpcLogDebug("[rpc][client] SendRequestForWithAttachment end, result:%d", result);
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
                auto protocols = rpc_protocol::GetSerializeProtocol(serialize_type);
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

    template<typename T>
    auto DeserializeRpcResult_(std::future<async_rpc_raw_result> &&future, coro::FutureAwaiter<async_rpc_raw_result>::FutureCallBack &&future_callback, std::weak_ptr<control_t> &&ctrl) -> ToolBox::coro::Task<async_rpc_result<T>, coro::NewThreadExecutor>
    {
        auto result = co_await coro::FutureAwaiter<async_rpc_raw_result>(std::move(future)).with_future_callback(std::move(future_callback));
        fprintf(stderr, "[rpc][client] DeserializeRpcResult_ result index: %zu\n", result.index());
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
            RpcLogDebug("[rpc][client] DeserializeRpcResult_ success");
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

    template<typename T>
    auto BuildFailedRpcResult_(CoroRpc::Errc Errc) -> ToolBox::coro::Task<async_rpc_result<T>, coro::NewThreadExecutor>
    {
        co_return async_rpc_result<T>{Errc};
    }

    template<auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::NewThreadExecutor> SendRequestForImpl_(auto duration, uint32_t &id, ToolBox::HTIMER &&timer, std::string_view attachment, Args &&...args)
    {
        using return_type = std::invoke_result_t<decltype(func), Args...>;

        // 检查定时器
        int32_t time_left = ToolBox::TimerMgr->GetTimeLeft(timer);
        if(time_left <= 0)
        {
            co_return CoroRpc::Errc::ERR_TIMEOUT;
        }
        co_return co_await SendImpl_<func>(id, attachment, std::forward<Args>(args)...);
    }

    template<auto func, typename... Args>
    ToolBox::coro::Task<CoroRpc::Errc, coro::NewThreadExecutor> SendImpl_(uint32_t &id, std::string_view attachment, Args &&...args)
    {
        std::string buffer;
        auto errc = PrepareBuffer_<func>(id, attachment.size(), buffer, std::forward<Args>(args)...);
        if (errc != CoroRpc::Errc::SUCCESS) {
            RpcLogError("[rpc][client] SendImpl_ PrepareBuffer_ failed, rpc_errc: %d", errc);
            co_return errc;
        }
        std::pair<std::error_code, std::size_t> send_result;
        int head_body_size = buffer.size();
        if (!attachment.empty()) 
        {
            // Append attachment data to buffer
            size_t original_size = buffer.size();
            buffer.resize(original_size + attachment.size());
            std::memcpy(buffer.data() + original_size, attachment.data(), attachment.size());
        }
        RpcLogDebug("[rpc][client] SendImpl_ buffer size: %zu, head_body_size: %d, attachment_size: %zu, buffer: %s", buffer.size(), head_body_size, attachment.size(), buffer.c_str());
        if(!send_callback_)
        {
            RpcLogError("[rpc][client] SendImpl_: send_callback_ not set");
            co_return CoroRpc::Errc::ERR_SEND_CALLBACK_NOT_SET;
        }
        send_callback_(std::move(buffer));
        // send buffer
        co_return CoroRpc::Errc::SUCCESS;
    }

    template<auto func, typename... Args>
    auto PrepareBuffer_(uint32_t &id, std::size_t attachment_size, std::string & buffer, Args &&...args) -> CoroRpc::Errc
    {
        rpc_func_key key{};
        if constexpr(HasGenRegisterKey<rpc_protocol, func>)
        {
            key = rpc_protocol::template GenRegisterKey<func>();
        } else {
            key = CoroRpcTools::AutoGenRegisterKey<func>();
        }
        RpcLogDebug("[rpc][client] RegisterOneHandler.auto_gen_key: key: %u, func: %s", key, ToolBox::GetFuncName<func>().data());

        CoroRpcProtocol::ReqHeader req_head;
        req_head = {};
        req_head.magic = rpc_protocol::magic_number;
        req_head.func_id = key;
        req_head.attach_length = attachment_size;
        id = ++request_id_;
        req_head.seq_num = id;

        auto serialize_proto = rpc_protocol::template GetSerializeProtocol(req_head); 
        if(!serialize_proto.has_value())
        {
            RpcLogError("[rpc][client] PrepareBuffer_: serialize protocol not supported");
            return CoroRpc::Errc::ERR_PROTOCOL_NOT_SUPPORTED;
        }
        CoroRpc::Errc error_code = CoroRpc::Errc::SUCCESS;
        return std::visit([&]<typename serialize_protocol>(const serialize_protocol& obj) -> CoroRpc::Errc {
            req_head.length = serialize_protocol::SerializeSize(std::forward<Args>(args)...);
            if(req_head.length > UINT32_MAX) [[unlikely]]
            {
                RpcLogError("[rpc][client] attachment is too long, max length is %u", UINT32_MAX);
                return CoroRpc::Errc::ERR_MESSAGE_TOO_LARGE;
            }
            buffer.resize(rpc_protocol::REQ_HEAD_LEN + req_head.length);
            memcpy(buffer.data(), &req_head, rpc_protocol::REQ_HEAD_LEN);
            error_code = serialize_protocol::SerializeToBuffer(buffer.data() + rpc_protocol::REQ_HEAD_LEN, buffer.size() - rpc_protocol::REQ_HEAD_LEN, std::forward<Args>(args)...);
            return error_code;
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
        RpcLogDebug("[rpc][client] OnRecvResp_ header: %s", header.ToString().c_str());
        std::string body;
        std::string attachment;
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

