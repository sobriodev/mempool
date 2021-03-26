cmake_minimum_required(VERSION 3.12)
project(mempool_src C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
option(BUILD_WITH_COVERAGE "Build the library with coverage results" OFF)
option(BUILD_FOR_UT "Build the library for unit testing" ON)

if(BUILD_WITH_COVERAGE)
    # Fix gcov linking errors
    link_libraries(gcov)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

include_directories(${mempool_SOURCE_DIR}/include)

add_library(mempool_src dll.c mempool.c)
target_compile_definitions(mempool_src PRIVATE MEMPOOL_CPU_ARCH=64)

# Library versions for unit testing
if(BUILD_FOR_UT)
    # Mempool version with sanity check enabled
    add_library(mempool_src_sanity_check dll.c mempool.c)
    target_compile_definitions(mempool_src_sanity_check PRIVATE
            MEMPOOL_CPU_ARCH=64
            DLL_NEW_NODE_SANITY_CHECK
            DLL_HEAD_SANITY_CHECK
            MEMPOOL_SANITY_CHECK)
endif()