#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__NT__)
    #define PLATFORM_WINDOWS
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOS__)
    #define PLATFORM_MACOS
#elif defined(__linux__) || defined(__linux)
    #define PLATFORM_LINUX
#else
    #define PLATFORM_UNKNOWN
#endif
