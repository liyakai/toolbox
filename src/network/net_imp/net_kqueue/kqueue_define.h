#pragma once

#if defined(__APPLE__)
// constexpr std::size_t MAXEVENTS = 8192;
constexpr std::size_t KQUEUE_WAIT_MSECONDS = 2;

#endif  // __APPLE__