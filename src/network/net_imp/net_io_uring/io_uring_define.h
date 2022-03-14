#pragma once

#ifdef __linux__

namespace ToolBox{

constexpr std::size_t MAX_MESSAGE_LEN = DEFAULT_RING_BUFF_SIZE;
constexpr std::size_t MAX_CONNECTIONS = MAX_SOCKET_COUNT;
constexpr std::size_t BUFFERS_COUNT = MAX_CONNECTIONS;
constexpr std::size_t URING_WAIT_MSECONDS = 2;

}

#endif  // __linux__
