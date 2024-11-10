#pragma once

#include "tools/log.h"
#include <cstdint>
#include <string_view>
#define RpcLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
#define RpcLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
#define RpcLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
#define RpcLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
#define RpcLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
#define RpcLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)


namespace ToolBox::coro_rpc
{
    enum class errc: uint16_t
    {
        SUCCESS = 0,
        ERR_TIMEOUT = 1,
        ERR_INVALID_ARGUMENTS = 2,
        ERR_FUNC_NOT_REGISTERED = 3,
        ERR_PROTOCOL = 4,
        ERR_OPERATION_CANCELED = 5,
        ERR_SERIAL_NUMBER_CONFLICT = 6,
        ERR_MESSAGE_TOO_LARGE = 7,
    };
}