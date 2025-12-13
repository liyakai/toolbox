#pragma once
#include "singleton.h"
#include <functional>
#include <stdarg.h>
#include <cstdio>
#include <mutex>
#include <vector>
// 实现日志模块
namespace ToolBox
{


    /*
    * 定义日志等级枚举
    */
    enum class LogLevel
    {
        LOG_MIN = 0,
        LOG_TRACE,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL,

        LOG_MAX,
    };

    // 定义日志宏   
#define LogTrace(LogFormat, ...) !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_TRACE) ? false : LogMgr->Trace("[TRACE] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LogDebug(LogFormat, ...) !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_DEBUG) ? false : LogMgr->Debug("[DEBUG] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LogInfo(LogFormat, ...)  !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_INFO)  ? false : LogMgr->Info("[INFO] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LogWarn(LogFormat, ...)  !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_WARN)  ? false : LogMgr->Warn("[WARN] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LogError(LogFormat, ...) !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_ERROR) ? false : LogMgr->Error("[ERROR] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define LogFatal(LogFormat, ...) !LogMgr->IsEnabled(ToolBox::LogLevel::LOG_FATAL) ? false : LogMgr->Fatal("[FATAL] " LogFormat " FUNC[%s] FILE[%s:%d]", ## __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)

    using LogCallback = std::function<void(LogLevel level, const char* fmt)>;

    /*
    * 定义日志管理器,初步管理日志等级,具体写日志可用原生或者第三方日志库[目前只实现了打印到屏幕]
    */
    class LogManager
    {
    public:
        /*
        * 设置日志等级
        */
        void SetLogLevel(LogLevel log_level)
        {
            log_level_ = log_level;
        }
        /*
        * @brief 日志等级是否开启
        */
        bool IsEnabled(LogLevel log_level) const
        {
            return log_level >= log_level_;
        }
        /*
        * @brief 设置日志回调
        */
        void SetLogCallback(const ToolBox::LogCallback& log_callback)
        {
            log_callback_ = log_callback;
        }
    public:
        bool Trace(const char* fmt, ...)
        {
            if (LogLevel::LOG_TRACE < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_TRACE, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }

        bool Debug(const char* fmt, ...)
        {
            if (LogLevel::LOG_DEBUG < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_DEBUG, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }


        bool Info(const char* fmt, ...)
        {
            if (LogLevel::LOG_INFO < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_INFO, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }


        bool Warn(const char* fmt, ...)
        {
            if (LogLevel::LOG_WARN < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_WARN, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }


        bool Error(const char* fmt, ...)
        {
            if (LogLevel::LOG_ERROR < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_ERROR, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }



        bool Fatal(const char* fmt, ...)
        {
            if (LogLevel::LOG_FATAL < log_level_)
            {
                return true;
            }
            va_list ap;
            va_start(ap, fmt);
            char* log_buffer = GetThreadLocalBuffer();
            vsnprintf(log_buffer, log_buffer_size_, fmt, ap);
            va_end(ap);
            {
                std::lock_guard<std::mutex> lock(print_mutex_);
                if (log_callback_)
                {
                    log_callback_(ToolBox::LogLevel::LOG_FATAL, log_buffer);
                }
                fprintf(stderr, "%s\n", log_buffer);
            }
            return true;
        }
    private:
        LogLevel log_level_ = LogLevel::LOG_DEBUG;    // 全局日志等级
        const static int log_buffer_size_ = 8 * 1024;  // 8KB 缓冲区，避免栈溢出
        LogCallback log_callback_;
        mutable std::mutex print_mutex_;  // 保护 fprintf 和 log_callback_ 的互斥锁
        
        // 获取线程本地缓冲区（使用动态分配避免栈溢出）
        static char* GetThreadLocalBuffer()
        {
            thread_local static std::vector<char> buffer(log_buffer_size_);
            return buffer.data();
        }
    };

#define LogMgr ToolBox::Singleton<ToolBox::LogManager>::Instance()
}; // namespace ToolBox