# cmake verson windows需要版本3的cmake. Linux可以降为版本2的cmake
cmake_minimum_required(VERSION 3.12.0)  

# project name
PROJECT(toolbox)

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11  -lpthread ")

# 设置编译调试信息
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG -g)
else()
    add_definitions(-O2)
endif()

# 头文件
INCLUDE_DIRECTORIES(
    .
    ./src/tools
    ./src/network
    ./src/net_work/network
	./src/net_work/network/net_imp
    ./src/net_work/network/net_imp/net_epoll
	./src/net_work/network/net_imp/net_iocp
    ./src/net_work/network/net_imp/net_kqueue
    ./src/net_work/network/net_imp/net_io_uring
    ./src/thirdparty
)

# 源文件
AUX_SOURCE_DIRECTORY(./src/tools DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/thirdparty/kcp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network/net_imp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network/net_imp/net_epoll DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network/net_imp/net_iocp DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network/net_imp/net_kqueue DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network/net_imp/net_io_uring DIR_SRCS)

# 设置输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set environment variable

# add executable file
# ADD_EXECUTABLE(${PROJECT_NAME} )

# add library
ADD_LIBRARY(toolbox ${DIR_SRCS})

# 单元测试
add_subdirectory(test)

# 编译选项
option(CMAKE_GPERF_TOOLS_CPU "Use Gperf Tools cpu-profiler" OFF)
if(CMAKE_GPERF_TOOLS_CPU)
    message(STATUS "Use Gperf Tools cpu-profiler")
    add_definitions(-DUSE_GPERF_TOOLS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lprofiler ")
    target_link_libraries(${PROJECT_NAME}  profiler.a)
endif()

option(CMAKE_GPERF_TOOLS_HEAP "Use Gperf Tools heap checker" OFF)
if(CMAKE_GPERF_TOOLS_HEAP)
    message(STATUS "Use Gperf Tools heap checker")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcmalloc ")
    target_link_libraries(${PROJECT_NAME} libtcmalloc.a )
endif()