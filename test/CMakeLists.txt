
# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++20")
message(STATUS "CMAKE_CXX_FLAGS -> ${CMAKE_CXX_FLAGS}")

# output 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


# 头文件
INCLUDE_DIRECTORIES(
    .
    ..
    ../include/
    ../src/thirdparty/
)
# 源文件
AUX_SOURCE_DIRECTORY(. TEST_SRC)
AUX_SOURCE_DIRECTORY(./unit_test_frame TEST_SRC)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
)

add_executable(test_tools ${TEST_SRC})

# 链接库
TARGET_LINK_LIBRARIES(test_tools -lpthread)
if(WIN32 OR MSVC)
    target_link_libraries(test_tools ${PROJECT_SOURCE_DIR}/lib/libtoolbox.lib)
elseif (UNIX)
    target_link_libraries(test_tools ${PROJECT_SOURCE_DIR}/lib/libtoolbox.a)
if(CMAKE_USE_LIBIOURING)
    target_link_libraries(test_tools ${PROJECT_SOURCE_DIR}/lib/liburing.a)
endif(CMAKE_USE_LIBIOURING)
endif()

# CPU评测工具
option(CMAKE_GPERF_TOOLS_CPU "Use Gperf Tools cpu-profiler" OFF)
if(CMAKE_GPERF_TOOLS_CPU)
    message(STATUS "Use Gperf Tools cpu-profiler")
    add_definitions(-DUSE_GPERF_TOOLS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lprofiler ")
    target_link_libraries(test_tools ${PROJECT_SOURCE_DIR}/lib/libprofiler.a)
endif()

# 内存泄漏评测工具
option(CMAKE_GPERF_TOOLS_HEAP "Use Gperf Tools heap checker" OFF)
if(CMAKE_GPERF_TOOLS_HEAP)
    message(STATUS "Use Gperf Tools heap checker")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcmalloc")
    target_link_libraries(test_tools libtcmalloc.a )
endif()

# 内存分配器 jemalloc
option(CMAKE_JEMALLOC "Use jemalloc" OFF)
if(CMAKE_JEMALLOC)
    message(STATUS "Use jemalloc")
    add_definitions(-DUSE_CMAKE_JEMALLOC)           # 使用 jemalloc
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ljemalloc -pthread")
    target_link_libraries(test_tools libjemalloc.a )
endif()

