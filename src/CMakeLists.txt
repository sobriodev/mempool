cmake_minimum_required(VERSION 3.12)
project(mempool_src C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")

include_directories(${mempool_SOURCE_DIR}/include)
add_library(mempool_src dll.c)