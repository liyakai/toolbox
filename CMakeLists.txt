# cmake verson windows需要版本3的cmake. Linux可以降为版本2的cmake
cmake_minimum_required(VERSION 3.12.0)  

# project name
PROJECT(toolbox)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(unity_build)
include(ccache)

# 编译库
add_subdirectory(src)
# 编译单元测试
add_subdirectory(test)



