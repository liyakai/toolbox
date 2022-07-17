
# output 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


# 头文件
INCLUDE_DIRECTORIES(
    .
    ..
    ../include/
    ../src/
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



