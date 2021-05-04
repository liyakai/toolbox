if(WIN32 OR MSVC)
    set(TEST_LIBS ${PROJECT_SOURCE_DIR}/lib/toolbox.lib)
elseif (UNIX)
    set(TEST_LIBS ${PROJECT_SOURCE_DIR}/lib/libtoolbox.a)
endif()


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(. TEST_SRC)
aux_source_directory(./tools/unit_test_frame TEST_SRC)

add_executable(test_tools ${TEST_SRC})

target_link_libraries(test_tools ${TEST_LIBS})
