
# 根据宏确定不同的操作系统
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
    add_definitions(-DOS_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOS")
    add_definitions(-DOS_MAC)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
    add_definitions(-DOS_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()


# 设置编译选项                                             遇到编译错误就停止编译
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wfatal-errors")

# 设置编译调试信息
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG -g -O0)
else()
    add_definitions(-O2)
endif()

# 头文件
INCLUDE_DIRECTORIES(
    .
    ..
    ../../include/
    ./network
	./network/net_imp
    ./network/net_imp/net_epoll
	./network/net_imp/net_iocp
    ./network/net_imp/net_kqueue
    ./network/net_imp/net_io_uring
    ../thirdparty
)

# 源文件
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
AUX_SOURCE_DIRECTORY(../thirdparty/kcp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_imp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_imp/net_epoll DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_imp/net_iocp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_imp/net_kqueue DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_imp/net_io_uring DIR_SRCS)
set_source_files_properties(${DIR_SRCS} PROPERTIES LANGUAGE CXX)
# 设置输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set environment variable

# add executable file
# ADD_EXECUTABLE(${PROJECT_NAME} )

# 指定库的路径
# link_directories(
#     ${PROJECT_SOURCE_DIR}/lib
# )


# add library
ADD_LIBRARY(toolbox ${DIR_SRCS})



# 编译选项

#第三方库 libiouring
option(CMAKE_USE_LIBIOURING "Use thirdparty libiouring" OFF)
if(CMAKE_USE_LIBIOURING)
    message(STATUS "Use thirdparty libiouring")
    add_definitions(-DLINUX_IO_URING)           # linux 下使用 io_uring
endif()

# CPU评测工具
option(CMAKE_GPERF_TOOLS_CPU "Use Gperf Tools cpu-profiler" OFF)
if(CMAKE_GPERF_TOOLS_CPU)
    message(STATUS "Use Gperf Tools cpu-profiler")
    add_definitions(-DUSE_GPERF_TOOLS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lprofiler ")
    target_link_libraries(${PROJECT_NAME}  profiler.a)
endif()

# 内存泄漏评测工具
option(CMAKE_GPERF_TOOLS_HEAP "Use Gperf Tools heap checker" OFF)
if(CMAKE_GPERF_TOOLS_HEAP)
    message(STATUS "Use Gperf Tools heap checker")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcmalloc ")
    target_link_libraries(${PROJECT_NAME} libtcmalloc.a )
endif()

