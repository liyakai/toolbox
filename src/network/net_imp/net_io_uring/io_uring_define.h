#pragma once

#ifdef __linux__

namespace ToolBox{

constexpr std::size_t MAX_MESSAGE_LEN = 2048;
constexpr std::size_t MAX_CONNECTIONS = MAX_SOCKET_COUNT;

}

#endif  // __linux__
