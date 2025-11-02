#pragma once

// Description: 安全调用函数，用于在异常情况下捕获信号和异常，并调用错误回调函数
// Author: https://github.com/liyakai
// Date: 2025-11-02
// Version: 1.0.0
// License: MIT
// Copyright (c) 2025 liyakai
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <functional>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <cstdio>

#if defined(PLATFORM_WINDOWS)
    #include <windows.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
    #include <setjmp.h>
    #include <signal.h>
    #include <csignal>
    #include <thread>
    #include <mutex>
    #include <atomic>
#endif

namespace ToolBox {

/**
 * @brief 错误回调函数类型
 * @param context 调用上下文（通常是插件名称或标识符）
 * @param error_msg 错误消息
 */
using ErrorCallback = std::function<void(const std::string& context, const std::string& error_msg)>;

/**
 * @brief 默认错误回调（使用 fprintf 打印到 stderr）
 */
inline void DefaultErrorCallback(const std::string& context, const std::string& error_msg) {
    fprintf(stderr, "SafeCall [%s]: %s\n", context.c_str(), error_msg.c_str());
}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)

/**
 * @brief Linux/macOS 平台的信号处理器
 * 使用线程局部存储，确保线程安全
 * 优化：全局信号处理只设置一次，减少系统调用开销
 */
class SignalGuard {
public:
    SignalGuard() = default;
    ~SignalGuard() = default;

    /**
     * @brief 设置信号处理并保存恢复点
     * @param context 调用上下文
     * @return 如果首次调用返回 0，信号捕获后返回 1
     */
    int Setup(const std::string& context) {
        context_ = context;
        signal_caught_ = false;
        
        // 确保全局信号处理已设置（只设置一次）
        EnsureGlobalSignalHandler();
        
        // 设置线程局部存储的跳转缓冲区指针
        thread_local_jump_buffer_ = &jump_buffer_;
        thread_local_context_ = &context_;
        thread_local_signal_caught_ = &signal_caught_;
        
        // 使用 sigsetjmp，保存信号掩码（第二个参数为 1）
        return sigsetjmp(jump_buffer_, 1);
    }

    /**
     * @brief 恢复线程局部状态（不清除全局信号处理，因为可能还有其他 SignalGuard 在使用）
     */
    void Restore() {
        // 不再恢复全局信号处理，因为它可能是共享的
        // 只清除线程局部引用（如果需要的话，可以在析构时做）
        // 这里不需要做任何操作，因为下一个 Setup 会重新设置线程局部变量
    }

    /**
     * @brief 检查是否捕获到信号
     */
    bool SignalCaught() const { return signal_caught_; }

    /**
     * @brief 获取上下文
     */
    const std::string& GetContext() const { return context_; }

private:
    /**
     * @brief 确保全局信号处理已设置（只设置一次，线程安全）
     */
    static void EnsureGlobalSignalHandler() {
        // 使用双重检查锁定模式，确保线程安全且高效
        if (!signal_handler_initialized_.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(init_mutex_);
            
            // 再次检查（可能在获取锁的过程中，其他线程已经初始化了）
            if (!signal_handler_initialized_.load(std::memory_order_relaxed)) {
                // 保存原始信号处理（用于未来可能的恢复）
                sigaction(SIGSEGV, nullptr, &global_old_act_segv_);
                sigaction(SIGFPE, nullptr, &global_old_act_fpe_);
                
                // 设置新的全局信号处理
                struct sigaction new_act;
                new_act.sa_handler = SignalHandler;
                sigemptyset(&new_act.sa_mask);
                new_act.sa_flags = SA_NODEFER;
                
                sigaction(SIGSEGV, &new_act, nullptr);
                sigaction(SIGFPE, &new_act, nullptr);
                
                // 标记为已初始化
                signal_handler_initialized_.store(true, std::memory_order_release);
            }
        }
    }

    /**
     * @brief 全局信号处理程序（所有线程共享）
     */
    static void SignalHandler(int sig) {
        // 通过线程局部存储找到当前线程的 SignalGuard 实例
        if (thread_local_signal_caught_ && thread_local_context_ && thread_local_jump_buffer_) {
            *thread_local_signal_caught_ = true;
            // 跳回安全点
            siglongjmp(*thread_local_jump_buffer_, 1);
        }
        // 如果没有活动的 SignalGuard，信号会继续（可能会导致程序崩溃）
        // 这是预期行为，因为只有在 SafeCall 内部才会设置线程局部变量
    }

    // 线程局部存储：每个线程独立的 SignalGuard 引用
    static thread_local sigjmp_buf* thread_local_jump_buffer_;
    static thread_local std::string* thread_local_context_;
    static thread_local bool* thread_local_signal_caught_;

    // 全局状态：信号处理是否已初始化
    static std::atomic<bool> signal_handler_initialized_;
    static std::mutex init_mutex_;
    
    // 保存的原始信号处理（用于未来可能的恢复，虽然当前不恢复）
    static struct sigaction global_old_act_segv_;
    static struct sigaction global_old_act_fpe_;

    // 实例成员变量
    sigjmp_buf jump_buffer_;
    bool signal_caught_ = false;
    std::string context_;
};

// 静态成员变量定义
// 线程局部存储
thread_local sigjmp_buf* SignalGuard::thread_local_jump_buffer_ = nullptr;
thread_local std::string* SignalGuard::thread_local_context_ = nullptr;
thread_local bool* SignalGuard::thread_local_signal_caught_ = nullptr;

// 全局状态（线程安全初始化）
std::atomic<bool> SignalGuard::signal_handler_initialized_(false);
std::mutex SignalGuard::init_mutex_;

// 保存的原始信号处理
struct sigaction SignalGuard::global_old_act_segv_;
struct sigaction SignalGuard::global_old_act_fpe_;

#endif // defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)

/**
 * @brief 安全调用函数模板
 * @tparam Func 可调用对象类型（函数指针、lambda、std::function 等）
 * @tparam Args 参数类型
 * @param func 要调用的函数
 * @param context 调用上下文（用于错误日志）
 * @param error_callback 错误回调函数（可选，默认使用 DefaultErrorCallback）
 * @param args 函数参数
 * @return std::pair<bool, ReturnType> 第一个元素表示是否成功，第二个元素是返回值
 * 
 * @example
 *   // 调用无参数无返回值函数
 *   auto result = SafeCall(func, "plugin1");
 *   if (!result.first) {
 *       // 调用失败
 *   }
 * 
 *   // 调用带参数和返回值的函数
 *   auto result = SafeCall([](int x) { return x * 2; }, "plugin1", nullptr, 42);
 *   if (result.first) {
 *       int value = result.second; // value = 84
 *   }
 */
// 辅助类型：用于处理 void 返回类型
template<typename T>
struct SafeCallResult {
    using type = T;
};

template<>
struct SafeCallResult<void> {
    using type = std::monostate;
};

// void 函数特化版本
template<typename Func, typename... Args>
auto SafeCall(Func&& func, 
              const std::string& context,
              ErrorCallback error_callback = DefaultErrorCallback,
              Args&&... args) 
    -> std::enable_if_t<std::is_void_v<decltype(func(std::forward<Args>(args)...))>, std::pair<bool, std::monostate>>
{
#if defined(PLATFORM_WINDOWS)
    // Windows 平台：仅捕获 C++ 异常
    try {
        func(std::forward<Args>(args)...);
        return {true, std::monostate{}};
    } catch (const std::exception& e) {
        if (error_callback) {
            error_callback(context, std::string("std::exception: ") + e.what());
        }
        return {false, std::monostate{}};
    } catch (...) {
        if (error_callback) {
            error_callback(context, "unknown exception");
        }
        return {false, std::monostate{}};
    }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
    // Linux/macOS 平台：捕获信号和异常
    SignalGuard guard;
    int jump_result = guard.Setup(context);
    
    if (jump_result == 0) {
        try {
            func(std::forward<Args>(args)...);
            guard.Restore();
            // 正常情况下，如果信号触发，siglongjmp 会跳转到 else 分支
            // 但如果 siglongjmp 没有跳转（不应该发生），在这里检查作为后备
            if (guard.SignalCaught()) {
                if (error_callback) {
                    error_callback(context, "caught signal (SIGSEGV/SIGFPE)");
                }
                return {false, std::monostate{}};
            }
            return {true, std::monostate{}};
        } catch (const std::exception& e) {
            guard.Restore();
            if (error_callback) {
                error_callback(context, std::string("std::exception: ") + e.what());
            }
            return {false, std::monostate{}};
        } catch (...) {
            guard.Restore();
            if (error_callback) {
                error_callback(context, "unknown exception");
            }
            return {false, std::monostate{}};
        }
    } else {
        guard.Restore();
        if (error_callback) {
            error_callback(context, "caught signal (SIGSEGV/SIGFPE)");
        }
        return {false, std::monostate{}};
    }
#endif
}

// 非 void 函数版本
template<typename Func, typename... Args>
auto SafeCall(Func&& func, 
              const std::string& context,
              ErrorCallback error_callback = DefaultErrorCallback,
              Args&&... args) 
    -> std::enable_if_t<!std::is_void_v<decltype(func(std::forward<Args>(args)...))>, 
                       std::pair<bool, decltype(func(std::forward<Args>(args)...))>>
{
    using ReturnType = decltype(func(std::forward<Args>(args)...));
    
#if defined(PLATFORM_WINDOWS)
    // Windows 平台：仅捕获 C++ 异常
    try {
        return {true, func(std::forward<Args>(args)...)};
    } catch (const std::exception& e) {
        if (error_callback) {
            error_callback(context, std::string("std::exception: ") + e.what());
        }
        return {false, ReturnType{}};
    } catch (...) {
        if (error_callback) {
            error_callback(context, "unknown exception");
        }
        return {false, ReturnType{}};
    }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
    // Linux/macOS 平台：捕获信号和异常
    SignalGuard guard;
    int jump_result = guard.Setup(context);
    
    if (jump_result == 0) {
        try {
            auto result = func(std::forward<Args>(args)...);
            guard.Restore();
            // 正常情况下，如果信号触发，siglongjmp 会跳转到 else 分支
            // 但如果 siglongjmp 没有跳转（不应该发生），在这里检查作为后备
            if (guard.SignalCaught()) {
                if (error_callback) {
                    error_callback(context, "caught signal (SIGSEGV/SIGFPE)");
                }
                return {false, std::move(result)};
            }
            return {true, std::move(result)};
        } catch (const std::exception& e) {
            guard.Restore();
            if (error_callback) {
                error_callback(context, std::string("std::exception: ") + e.what());
            }
            return {false, ReturnType{}};
        } catch (...) {
            guard.Restore();
            if (error_callback) {
                error_callback(context, "unknown exception");
            }
            return {false, ReturnType{}};
        }
    } else {
        guard.Restore();
        if (error_callback) {
            error_callback(context, "caught signal (SIGSEGV/SIGFPE)");
        }
        return {false, ReturnType{}};
    }
#endif
}

/**
 * @brief 简化版本：只返回成功/失败，忽略返回值
 * 适用于无返回值函数或不需要返回值的场景
 */
template<typename Func, typename... Args>
bool SafeCallSimple(Func&& func,
                    const std::string& context,
                    ErrorCallback error_callback = DefaultErrorCallback,
                    Args&&... args) 
{
    auto result = SafeCall(std::forward<Func>(func), context, error_callback, std::forward<Args>(args)...);
    return result.first;
}

} // namespace ToolBox

