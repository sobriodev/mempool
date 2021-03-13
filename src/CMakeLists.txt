cmake_minimum_required(VERSION 3.12)
project(mempool_src C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
option(BUILD_WITH_COVERAGE "Build the library with coverage results" OFF)

if(BUILD_WITH_COVERAGE)
    # Fix gcov linking errors
    link_libraries(gcov)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif(BUILD_WITH_COVERAGE)

include_directories(${mempool_SOURCE_DIR}/include)

# Library version without sanity check enabled
add_library(mempool_src dll.c mempool.c)

# Library version with sanity check enabled
add_library(mempool_src_sanity_check dll.c)
target_compile_definitions(mempool_src_sanity_check PRIVATE -D DLL_NEW_NODE_SANITY_CHECK)
target_compile_definitions(mempool_src_sanity_check PRIVATE -D DLL_HEAD_SANITY_CHECK)