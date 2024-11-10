# cmake verson windows需要版本3的cmake. Linux可以降为版本2的cmake
cmake_minimum_required(VERSION 3.12.0)  

# project name
PROJECT(toolbox)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# include(unity_build)
include(ccache)

#clang gcc-toolchain
option(GCC_TOOLCHAIN "GCC_TOOLCHAIN" OFF)
if(GCC_TOOLCHAIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --gcc-toolchain=${GCC_TOOLCHAIN}")
endif()

set(CMAKE_CXX_STANDARD 20)

# 设置编译器为clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# 编译库
add_subdirectory(src)
# 编译单元测试
add_subdirectory(test)



