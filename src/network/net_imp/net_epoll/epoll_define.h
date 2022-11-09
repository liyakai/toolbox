#pragma once

#ifdef __linux__

namespace ToolBox{

constexpr std::size_t INVALID_SOCKET = -1;
constexpr std::size_t EPOLL_WAIT_MSECONDS = 2;

};  // ToolBox

#endif  // __linux__
