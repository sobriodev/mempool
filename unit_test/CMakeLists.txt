cmake_minimum_required(VERSION 3.15)
project(mempool_ut CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

SET(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPPUTEST_DIR ${CURRENT_DIR}/lib/cpputest)
SET(MAIN_DIR ${mempool_SOURCE_DIR})

# Enable memory leaks detection for cpputest
SET(CMAKE_CXX_FLAGS "-include ${CPPUTEST_DIR}/include/CppUTest/MemoryLeakDetectorMallocMacros.h")

# Include directories
include_directories(${MAIN_DIR}/include)
include_directories(${CPPUTEST_DIR}/include)

# Library directories
link_directories(${CPPUTEST_DIR})

# Executables
add_executable(Test TestRunner.cpp Test.cpp)
target_link_libraries(Test mempool_src CppUTest CppUTestExt)

# Test suites
add_test(NAME Test COMMAND Test -v)
