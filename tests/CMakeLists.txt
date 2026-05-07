add_library(unity STATIC
${CMAKE_CURRENT_SOURCE_DIR}/../vendor/unity/unity.c
)

target_include_directories(unity PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/../vendor/unity
)

add_executable(test_cbuf
    test_cbuf.c
)

target_link_libraries(test_cbuf
    PRIVATE
    cbuf
    unity
)


add_test(
    NAME cbuf_unit_test
    COMMAND test_cbuf
)