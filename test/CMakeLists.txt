
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
    ../3rdparty/
)
# 源文件
AUX_SOURCE_DIRECTORY(. TEST_SRC)
AUX_SOURCE_DIRECTORY(./unit_test_frame TEST_SRC)
AUX_SOURCE_DIRECTORY(./protobuf TEST_SRC)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/3rdparty/gperf_2_16
)
set(TARGET_NAME test_tools)
add_executable(${TARGET_NAME} ${TEST_SRC})

# 链接库
# 设置 protobuf 和 abseil 的路径
set(PROTOBUF_ROOT ${PROJECT_SOURCE_DIR}/3rdparty/protobuf_3.21.11)

TARGET_LINK_LIBRARIES(${TARGET_NAME} -lpthread)
if(WIN32 OR MSVC)
    target_link_libraries(${TARGET_NAME} ${PROJECT_SOURCE_DIR}/lib/libtoolbox.lib)
elseif (UNIX)
    # 首先获取所有 abseil 库文件
    file(GLOB ABSL_LIBRARIES "${PROTOBUF_ROOT}/lib/libabsl_*.a")
    target_link_libraries(${TARGET_NAME} 
        ${PROJECT_SOURCE_DIR}/lib/libtoolbox.a
        ${PROTOBUF_ROOT}/lib/libprotobuf.a
        # protobuf 依赖的 absl 库
        ${ABSL_LIBRARIES}  # 使用通配符替换所有 abseil 库
        -lz
        -pthread
    )
if(CMAKE_USE_LIBIOURING)
    target_link_libraries(${TARGET_NAME} ${PROJECT_SOURCE_DIR}/lib/liburing.a)
endif(CMAKE_USE_LIBIOURING)

endif()

# CPU评测工具
option(CMAKE_GPERF_TOOLS_CPU "Use Gperf Tools cpu-profiler" OFF)
if(CMAKE_GPERF_TOOLS_CPU)
    message(STATUS "Use Gperf Tools cpu-profiler")
    add_definitions(-DUSE_GPERF_TOOLS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lprofiler ")
    target_link_libraries(${TARGET_NAME} ${PROJECT_SOURCE_DIR}/lib/libprofiler.a)
endif()

# 内存泄漏评测工具
option(CMAKE_GPERF_TOOLS_MEM "Use tcmalloc_and_profile" OFF)
if(CMAKE_GPERF_TOOLS_MEM)
    message(STATUS "Use tcmalloc_and_profile")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcmalloc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    target_link_libraries(${TARGET_NAME} libtcmalloc_and_profiler.a )
endif()

# 使用 tcmalloc 内存分配器
option(CMAKE_TCMALLOC "Use tcmalloc" OFF)
if(CMAKE_TCMALLOC)
    message(STATUS "Use tcmalloc")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltcmalloc")
    target_link_libraries(${TARGET_NAME} libtcmalloc.a )
endif()

# 内存分配器 jemalloc
option(CMAKE_JEMALLOC "Use jemalloc" OFF)
if(CMAKE_JEMALLOC)
    message(STATUS "Use jemalloc")
    add_definitions(-DUSE_CMAKE_JEMALLOC)           # 使用 jemalloc
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ljemalloc -pthread")
    target_link_libraries(${TARGET_NAME} libjemalloc.a )
endif()

