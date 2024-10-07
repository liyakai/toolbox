#pragma once

#include "tools/log.h"

#define RpcLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
#define RpcLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
#define RpcLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
#define RpcLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
#define RpcLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
#define RpcLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)