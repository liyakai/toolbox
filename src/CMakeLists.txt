
# 根据宏确定不同的操作系统
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()


# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++20")
message(STATUS "CMAKE_CXX_FLAGS -> ${CMAKE_CXX_FLAGS}")

# 设置编译调试信息
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG -g -O0)
else()
    add_definitions(-O2)
    add_definitions(-DNDEBUG)
    message(STATUS "definitions -> -O2 -DNDEBUG")
endif()
message(STATUS "CMAKE_BUILD_TYPE -> ${CMAKE_BUILD_TYPE}")
# 头文件
INCLUDE_DIRECTORIES(
    .
    ../include/
    ../3rdparty/
    ./tools
    ./network
	./net_work/net_imp
    ./net_work/net_imp/net_epoll
	./net_work/net_imp/net_iocp
    ./net_work/net_imp/net_kqueue
    ./net_work/net_imp/net_io_uring
)

# 源文件
AUX_SOURCE_DIRECTORY(./tools DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network DIR_SRCS)
AUX_SOURCE_DIRECTORY(./thirdparty/kcp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network/net_imp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network/net_imp/net_epoll DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network/net_imp/net_iocp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network/net_imp/net_kqueue DIR_SRCS)
AUX_SOURCE_DIRECTORY(./network/net_imp/net_io_uring DIR_SRCS)
AUX_SOURCE_DIRECTORY(../3rdparty/kcp DIR_SRCS)

# 设置输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set environment variable

# add executable file
# ADD_EXECUTABLE(${PROJECT_NAME} )

# 指定库的路径
# link_directories(
#     ${PROJECT_SOURCE_DIR}/lib
# )

# 启用位置无关代码（Position Independent Code），用于生成共享库
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add library
ADD_LIBRARY(toolbox ${DIR_SRCS})



# 编译选项

#第三方库 libiouring
# [实验] CMAKE_USE_LIBIOURING 默认 OFF.开启后 TCP 走 io_uring,但 DelEvent/CQE 生命周期未闭环,
# 关闭连接后可能 UAF,禁止用于生产.ASYNC_CANCEL 闭环另议(C1-b).
option(CMAKE_USE_LIBIOURING "Use experimental io_uring backend (NOT for production)" OFF)
if(CMAKE_USE_LIBIOURING)
    message(WARNING "CMAKE_USE_LIBIOURING=ON: experimental backend, DelEvent/CQE UAF not closed-loop; do NOT use in production")
    add_definitions(-DLINUX_IO_URING)           # linux 下使用 io_uring(实验)
endif()

