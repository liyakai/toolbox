
# output 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置编译选项
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-attributes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall  -Wno-attributes -Wno-unused-variable") #-Wfatal-errors

# 指定库的路径
link_directories(
    ${PROJECT_SOURCE_DIR}/lib
)

# 头文件
INCLUDE_DIRECTORIES(
    .
    ./lib/
    ./ospfd/
)
# 源文件
AUX_SOURCE_DIRECTORY(. QUAGGA_SRC)
AUX_SOURCE_DIRECTORY(./lib QUAGGA_SRC)
AUX_SOURCE_DIRECTORY(./lib/printf QUAGGA_SRC)
AUX_SOURCE_DIRECTORY(./ospfd QUAGGA_SRC)
set_source_files_properties(${QUAGGA_SRC} PROPERTIES LANGUAGE C)


# 宏定义
#add_definitions(-DFRR_AUTOCONF_ATOMIC=On)
add_definitions(-DHAVE___ATOMIC=On)
add_definitions(-DHAVE_CONFIG_H=On)
add_definitions(-DFRR_DEFINE_DESC_TABLE=On)
#add_definitions(-DMULTIPATH_NUM=1024) # Maximum number of paths for a route
add_definitions(-DIOV_MAX=16) # 	/* IOV_MAX are normally defined in <sys/uio.h> , Posix.1g. example: Solaris2.6 are defined IOV_MAX size at 16.     */
add_definitions(-DSYSCONFDIR="/etc/frr")

# 链接库
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ljson-c -pthread -lfrrcares -ldl -rdynamic -lyang -lm -lunwind -lcap  -lcrypt")

add_executable(quagga ${QUAGGA_SRC})

target_link_libraries(quagga libfrrcares.a)
#target_link_libraries(quagga libnetsnmp.a)