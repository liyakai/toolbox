#pragma once
#include <chrono>
#include <ctime>

namespace ToolBox{

/// 获取毫秒级时间戳(ms)
static inline std::time_t GetMillSecondTimeStamp()
{
    auto point = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto duraion = std::chrono::duration_cast<std::chrono::milliseconds>(point.time_since_epoch());
    return duraion.count();
}

};  // ToolBox