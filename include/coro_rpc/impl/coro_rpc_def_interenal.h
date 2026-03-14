#pragma once

#include "tools/log.h"
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <string_view>
#include <stdexcept>
#include "tools/function_name.h"
#include "tools/md5.h"
#include "tools/cpp20_coroutine.h"

#define RpcLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
#define RpcLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
#define RpcLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
#define RpcLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
#define RpcLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
#define RpcLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)


namespace ToolBox::CoroRpc
{
    enum class Errc: uint16_t
    {
        SUCCESS = 0,
        ERR_TIMEOUT = 1,
        ERR_INVALID_ARGUMENTS = 2,
        ERR_FUNC_NOT_REGISTERED = 3,
        ERR_PROTOCOL = 4,
        ERR_OPERATION_CANCELED = 5,
        ERR_SERIAL_NUMBER_CONFLICT = 6,
        ERR_MESSAGE_TOO_LARGE = 7,
        ERR_BUFFER_TOO_SMALL = 8,
        ERR_HANDLER_THROW_EXCEPTION = 9,
        ERR_SEND_CALLBACK_NOT_SET = 10,
        ERR_SERVER_PREPARE_HEADER_FAILED = 11,
        ERR_SERIALIZE_FAILED = 12,
        ERR_DESERIALIZE_FAILED = 13,
        ERR_PROTOCOL_NOT_SUPPORTED = 14,
    };

    
class CoroRpcTools
{
public:
    using rpc_func_key_t = uint32_t;
    using rpc_func_key = rpc_func_key_t;
    // 基础类型的序列化（带边界检查）
    template<typename T>
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, T &&arg)
    {
        // 确保缓冲区有足够空间
        if (offset + sizeof(T) > buffer.size()) {
            buffer.resize(offset + sizeof(T));
        }
        const T& value = arg;
        const std::byte*  p_bytes = reinterpret_cast<const std::byte*>(&value);
        std::copy(p_bytes, p_bytes + sizeof(T), buffer.begin() + offset);
        offset += sizeof(T);
    }

    // 字符串类型的特化版本（带边界检查）
    static void SerializeArg(std::vector<std::byte> &buffer, std::size_t &offset, std::string str)
    {
        // 先序列化字符串长度
        uint32_t length = static_cast<uint32_t>(str.size());
        SerializeArg(buffer, offset, length);

        // 确保 buffer 有足够的空间（包括字符串内容）
        if(offset + length > buffer.size())
        {
            buffer.resize(offset + length);
        }
        // 再序列化字符串内容
        const std::byte* bytes = reinterpret_cast<const std::byte*>(str.data());
        std::copy(bytes, bytes + length, buffer.begin() + offset);
        offset += length;
    }

    // 基础类型的反序列化（带边界检查）
    template<typename T>
    static T DeserializeArg(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        // 检查缓冲区边界
        if (offset + sizeof(T) > buffer.size()) {
            throw std::runtime_error("DeserializeArg: buffer overflow");
        }
        T value;
        std::byte* p_bytes = reinterpret_cast<std::byte*>(&value);
        std::copy(buffer.begin() + offset, buffer.begin() + offset + sizeof(T), p_bytes);
        offset += sizeof(T);
        return value;
    }

    // 字符串类型的反序列化特化版本（带边界检查）
    static std::string DeserializeString(const std::vector<std::byte> &buffer, std::size_t &offset)
    {
        // 先反序列化字符串长度
        uint32_t length = DeserializeArg<uint32_t>(buffer, offset);

        // 检查缓冲区边界
        if (offset + length > buffer.size()) {
            throw std::runtime_error("DeserializeString: buffer overflow");
        }
        // 反序列化字符串内容
        std::string result(reinterpret_cast<const char*>(buffer.data() + offset), length);
        offset += length;
        return result;
    }

    template<auto func>
    static constexpr rpc_func_key AutoGenRegisterKey()
    {
        constexpr auto name = ToolBox::GetFuncName<func>();
        auto id = MD5Hash32Constexpr(name);
        return static_cast<rpc_func_key>(id);
    }
};


// --------- 检查是否是 tuple_like 类型 ---------
template<typename T, typename = void>
struct is_tuple_like : std::false_type {};

template<typename T>
struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>> 
    : std::true_type {};

template<typename T>
inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;
// ---------------------------------------------

// --------- 从 Task<T> 或 StreamGenerator<T> 中提取内部类型 T ---------
template<typename T>
struct unwrap_task_return_type {
    using type = T;
};

template<typename R, typename Executor>
struct unwrap_task_return_type<ToolBox::coro::Task<R, Executor>> {
    using type = R;
};

// 前向声明 StreamGenerator
template<typename T>
class StreamGenerator;

// 特化：从 StreamGenerator<T> 中提取 T（在命名空间内部，可以直接使用 StreamGenerator）
template<typename T>
struct unwrap_task_return_type<StreamGenerator<T>> {
    using type = T;
};
// ---------------------------------------------


// ==================== 线程安全策略 ====================
// 
// 用于将锁类型模板化：单线程场景使用 NullMutex（零开销），
// 多线程场景使用 std::mutex（真正的互斥）。
//
// 使用方式：
//   CoroRpcClient<Protocol, ThreadUnsafe>  // 单线程，无锁（默认）
//   CoroRpcClient<Protocol, ThreadSafe>    // 多线程，有锁
//   CoroRpcServer<Protocol, map_t, ThreadUnsafe>  // 单线程，无锁（默认）
//   CoroRpcServer<Protocol, map_t, ThreadSafe>    // 多线程，有锁
// ===========================================================

/// 空互斥锁：满足 C++ Lockable 要求，所有操作为 no-op
struct NullMutex {
    void lock() noexcept {}
    void unlock() noexcept {}
    bool try_lock() noexcept { return true; }
};

/// 空条件变量：默认与无锁互斥配合使用，所有操作为 no-op
struct NullConditionVariable {
    void notify_one() noexcept {}
    void notify_all() noexcept {}

    template<typename LockType>
    void wait(std::unique_lock<LockType>&) noexcept {}

    template<typename LockType, typename Pred>
    void wait(std::unique_lock<LockType>&, Pred pred) noexcept(noexcept(pred())) {
        // 单线程策略下不真正阻塞，但仍应执行一次谓词，
        // 以尽量贴近 std::condition_variable::wait(lock, pred) 的语义。
        (void)pred();
    }
};

/// 非原子 bool：单线程下替代 std::atomic<bool>，零开销
/// 仅实现代码中实际使用到的接口子集
struct NullAtomicBool {
    bool value_ = false;

    NullAtomicBool() noexcept = default;
    NullAtomicBool(bool v) noexcept : value_(v) {}   // NOLINT: 允许隐式转换以兼容 std::atomic<bool> 的接口
    NullAtomicBool& operator=(bool v) noexcept { value_ = v; return *this; }

    operator bool() const noexcept { return value_; }   // NOLINT: 故意允许隐式转换
    bool load(std::memory_order = std::memory_order_seq_cst) const noexcept { return value_; }
    void store(bool v, std::memory_order = std::memory_order_seq_cst) noexcept { value_ = v; }
    bool compare_exchange_strong(bool& expected, bool desired,
                                  std::memory_order = std::memory_order_seq_cst) noexcept {
        if (value_ == expected) { value_ = desired; return true; }
        expected = value_;
        return false;
    }
};

/// 非原子 uint32_t：单线程下替代 std::atomic<uint32_t>
struct NullAtomicUint32 {
    uint32_t value_ = 0;

    NullAtomicUint32() noexcept = default;
    NullAtomicUint32(uint32_t v) noexcept : value_(v) {}   // NOLINT: 允许隐式转换以兼容 std::atomic<uint32_t> 的接口
    NullAtomicUint32& operator=(uint32_t v) noexcept { value_ = v; return *this; }

    operator uint32_t() const noexcept { return value_; }   // NOLINT
    uint32_t load(std::memory_order = std::memory_order_seq_cst) const noexcept { return value_; }
    void store(uint32_t v, std::memory_order = std::memory_order_seq_cst) noexcept { value_ = v; }
    uint32_t fetch_add(uint32_t v, std::memory_order = std::memory_order_seq_cst) noexcept {
        uint32_t old = value_; value_ += v; return old;
    }
    uint32_t operator++() noexcept { return ++value_; }
    uint32_t operator++(int) noexcept { return value_++; }
    uint32_t operator--() noexcept { return --value_; }
    uint32_t operator--(int) noexcept { return value_--; }
};

/// 非原子 size_t：单线程下替代 std::atomic<size_t>
struct NullAtomicSize {
    size_t value_ = 0;

    NullAtomicSize() noexcept = default;
    NullAtomicSize(size_t v) noexcept : value_(v) {}
    NullAtomicSize& operator=(size_t v) noexcept { value_ = v; return *this; }

    operator size_t() const noexcept { return value_; }
    size_t load(std::memory_order = std::memory_order_seq_cst) const noexcept { return value_; }
    void store(size_t v, std::memory_order = std::memory_order_seq_cst) noexcept { value_ = v; }
    size_t fetch_add(size_t v, std::memory_order = std::memory_order_seq_cst) noexcept {
        size_t old = value_; value_ += v; return old;
    }
    size_t fetch_sub(size_t v, std::memory_order = std::memory_order_seq_cst) noexcept {
        size_t old = value_; value_ -= v; return old;
    }
};

/// 通用线程策略：把锁/条件变量/原子类型都做成模板参数。
template <typename MutexType,
          typename ConditionVariableType,
          typename AtomicBoolType,
          typename AtomicUint32Type,
          typename AtomicSizeType,
          bool EnableAsyncPumpWorker>
struct ThreadPolicy {
    using mutex_type = MutexType;
    using condition_variable_type = ConditionVariableType;
    using atomic_bool_type = AtomicBoolType;
    using atomic_uint32_type = AtomicUint32Type;
    using atomic_size_type = AtomicSizeType;
    static constexpr bool enable_async_pump_worker = EnableAsyncPumpWorker;
};

/// 线程安全策略：真正的互斥锁
using ThreadSafe = ThreadPolicy<std::mutex,
                                std::condition_variable,
                                std::atomic<bool>,
                                std::atomic<uint32_t>,
                                std::atomic<size_t>,
                                true>;

/// 线程不安全策略（默认）：空锁，零开销
using ThreadUnsafe = ThreadPolicy<NullMutex,
                                  NullConditionVariable,
                                  NullAtomicBool,
                                  NullAtomicUint32,
                                  NullAtomicSize,
                                  false>;

}   // namespace ToolBox::CoroRpc
