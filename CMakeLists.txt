# cmake verson
cmake_minimum_required(VERSION 2.8)

# project name
PROJECT(toolbox)

# flat
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Debug
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG -g)
endif()

# head file path
INCLUDE_DIRECTORIES(
    .
    ./tools
    ./net_epoll
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# source directory
AUX_SOURCE_DIRECTORY(./tools DIR_SRCS)
AUX_SOURCE_DIRECTORY(./test DIR_SRCS)
AUX_SOURCE_DIRECTORY(./tools/unit_test_frame DIR_SRCS)
AUX_SOURCE_DIRECTORY(./net_epoll DIR_SRCS)

# set environment variable

# add executable file
# ADD_EXECUTABLE(${PROJECT_NAME} )

# add library
ADD_LIBRARY(toolbox ${DIR_SRCS})

add_subdirectory(test)
