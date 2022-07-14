# cmake verson windows需要版本3的cmake. Linux可以降为版本2的cmake
cmake_minimum_required(VERSION 3.12.0)  

# project name
PROJECT(toolbox)

add_subdirectory(src)
# 单元测试
add_subdirectory(test)



