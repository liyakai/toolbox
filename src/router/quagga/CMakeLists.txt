
# output 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置编译选项
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-attributes")
# 链接库
#target_link_libraries(${PROJECT_NAME} libssl.a )

# 头文件
INCLUDE_DIRECTORIES(
    .
    ./lib/
    ./ospfd/
)
# 源文件
AUX_SOURCE_DIRECTORY(. QUAGGA_SRC)
AUX_SOURCE_DIRECTORY(./lib QUAGGA_SRC)
AUX_SOURCE_DIRECTORY(./ospfd QUAGGA_SRC)
set_source_files_properties(${QUAGGA_SRC} PROPERTIES LANGUAGE CXX)


# 宏定义
add_definitions(-DFRR_AUTOCONF_ATOMIC=On)
#add_definitions(-DHAVE___ATOMIC=On)
add_definitions(-DMULTIPATH_NUM=1024) # Maximum number of paths for a route




add_executable(quagga ${QUAGGA_SRC})

