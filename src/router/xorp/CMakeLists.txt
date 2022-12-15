
# output 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置编译选项                            消除 throw 警告
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -lcrypto")

# 链接库
#target_link_libraries(${PROJECT_NAME} libssl.a )

# 头文件
INCLUDE_DIRECTORIES(
    .
    ./libxorp/
    ./ospf/
)
# 源文件
AUX_SOURCE_DIRECTORY(. XORP_SRC)
AUX_SOURCE_DIRECTORY(./bgp XORP_SRC)
AUX_SOURCE_DIRECTORY(./libcomm XORP_SRC)
AUX_SOURCE_DIRECTORY(./libfeaclient XORP_SRC)
AUX_SOURCE_DIRECTORY(./libproto XORP_SRC)
AUX_SOURCE_DIRECTORY(./libxipc XORP_SRC)
AUX_SOURCE_DIRECTORY(./libxorp XORP_SRC)
AUX_SOURCE_DIRECTORY(./ospf XORP_SRC)
AUX_SOURCE_DIRECTORY(./policy XORP_SRC)
AUX_SOURCE_DIRECTORY(./policy/backend XORP_SRC)
AUX_SOURCE_DIRECTORY(./policy/common XORP_SRC)
AUX_SOURCE_DIRECTORY(./xrl/interfaces XORP_SRC)
AUX_SOURCE_DIRECTORY(./xrl/targets XORP_SRC)


add_definitions(-DXRL_PF='x')
add_definitions(-DPROFILE_UTILS_REQUIRED)



add_executable(ospf ${XORP_SRC})

