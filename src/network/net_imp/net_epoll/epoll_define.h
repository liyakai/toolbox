#pragma once

#ifdef __linux__

constexpr std::size_t INVALID_SOCKET = -1;
constexpr std::size_t MAXEVENTS = 8192; /* epoll_create参数 */
constexpr std::size_t EPOLL_WAIT_MSECONDS = 2;

#endif  // __linux__
