cmake_minimum_required(VERSION 3.12)
project(mempool_src C)

set(CMAKE_C_STANDARD 99)

include_directories(${mempool_SOURCE_DIR}/include)
add_library(mempool_src dll.c)