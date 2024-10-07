#pragma once

#include "tools/timer.h"
#include <future>
#include "tools/cpp20_coroutine.h"
#include "coro_rpc_def_interenal.h"
namespace ToolBox {
namespace coro_rpc {

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
    std::string read_buffer_;
    std::string resp_attachment_buf_;
};

template<typename T>
struct async_rpc_result_value_t{
public:
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
    resp_body buffer_;

    std::string_view get_attachment() const noexcept { return buffer_.resp_attachment_buf_; }
    resp_body release_buffer() noexcept { return std::move(buffer_); }

};

template<typename T>
using async_rpc_result = async_rpc_result_value_t<T>;

template<typename T>
using async_rpc_type_t = typename rpc_return_type<T>::type;
class CoroRpcClient {
public:
    struct Config {
        uint64_t client_id = get_global_client_id();
        std::chrono::milliseconds timeout = std::chrono::seconds(10);
    };

    CoroRpcClient(uint64_t client_id = get_global_client_id()) {
        config_.client_id = client_id;
    }
    ~CoroRpcClient() = default;

    // 获取配置
    const Config &get_config() const { return config_; }

    uint32_t get_client_id() const { return config_.client_id; }

    // call RPC with default timeout(5s)
    template <auto func, typename... Args>
    auto call(Args &&...args) -> std::invoke_result_t<decltype(func), Args...> {
        return call_for<func>(std::chrono::seconds(5), std::forward<Args>(args)...);
    }

    // call RPC with timeout
    template <auto func, typename... Args>
    auto call_for(auto duration, Args &&...args)
        -> ToolBox::coro::Task<std::invoke_result_t<decltype(func), Args...>> {
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        auto async_result =
            co_await co_await send_request_for_with_attachment<func, Args...>(
                duration, req_attachment_, std::forward<Args>(args)...);
        req_attachment_ = {};
        if (async_result) {
        if constexpr (std::is_void_v<return_type>) {
            co_return {};
        } else {
            co_return std::move(async_result->result());
        }
        } else {
        co_return {};
        }
    }

    bool set_req_attachment(std::string_view attachment) {
        if (attachment.size() > UINT32_MAX) {
        RpcLogError("[rpc][client] attachment is too long, max length is {}",
                    UINT32_MAX);
        return false;
        }
        req_attachment_ = attachment;
        return true;
    }

    template <auto func, typename... Args>
    auto send_request_for_with_attachment(
        auto duration, std::string_view attachment,
        Args &&...args) -> std::invoke_result_t<decltype(func), Args...> {
        using return_type = std::invoke_result_t<decltype(func), Args...>;
        uint32_t id;
                auto timer = std::make_unique<ToolBox::TimerWheel>(duration, [this, id]() {
            });
        }

private:
    auto timeout(auto duration, std::string error_msg)
    {
      co_return false;
    }

private:
    struct async_rpc_raw_result_value_type{
        resp_body buffer_;
        uint8_t errc_;
    };
    using async_rpc_raw_result = std::variant<async_rpc_raw_result_value_type, std::error_code>;
    struct handler_t
    {
      std::unique_ptr<ToolBox::TimerWheel> timer_;
      std::promise<async_rpc_raw_result> promise_;
      handler_t(std::unique_ptr<ToolBox::TimerWheel> && timer,
                std::promise<async_rpc_raw_result> && promise)
          : timer_(std::move(timer)), promise_(std::move(promise)) {}
      void operator()(resp_body &&buffer, uint8_t rpc_errc) {
        // TODO
      }
      void local_error(std::error_code ec) {
        // TODO
      }
    };
    struct control_t
    {
      bool is_timeout_ = false;
      std::unordered_map<uint32_t, handler_t> response_handler_table_;
      control_t(bool is_timeout) : is_timeout_(is_timeout) {}

    };
private:
    uint64_t client_id_;
    bool should_reset_ = false;
    std::unique_ptr<ToolBox::TimerWheel> timer_;
    std::unique_ptr<control_t> control_;
    std::string_view req_attachment_;
    Config config_;
};

}; // namespace coro_rpc
}; // namespace ToolBox

