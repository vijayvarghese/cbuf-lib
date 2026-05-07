cmake_minimum_required(VERSION 3.20)
project(cbuf VERSION 1.0.0 LANGUAGES C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

add_library(cbuf STATIC
    src/cbuf.c
)

target_include_directories(cbuf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_subdirectory(tests)
endif()