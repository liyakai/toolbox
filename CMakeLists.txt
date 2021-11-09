# cmake verson
cmake_minimum_required(VERSION 3.18)

# project name
PROJECT(toolbox)

# flat
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11  -lpthread  -Wl,--no-as-needed")

# Debug
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG -g)
endif()

# head file path
INCLUDE_DIRECTORIES(
    .
    ./src/tools
    ./src/net_epoll
    ./src/network
    ./src/thirdparty
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# source directory
AUX_SOURCE_DIRECTORY(./src/tools DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/net_epoll DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/network DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/thirdparty/kcp DIR_SRCS)

# set environment variable

# add executable file
# ADD_EXECUTABLE(${PROJECT_NAME} )

# add library
ADD_LIBRARY(toolbox ${DIR_SRCS})

add_subdirectory(test)

option(CMAKE_GPERF_TOOLS "Use Gperf Tools" OFF)
if(CMAKE_GPERF_TOOLS)
    add_definitions(-DUSE_GPERF_TOOLS)
    target_link_libraries(${PROJECT_NAME}   profiler.a)
endif()