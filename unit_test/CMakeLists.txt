cmake_minimum_required(VERSION 3.12)
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
add_executable(TestDll TestRunner.cpp TestDll.cpp)
target_link_libraries(TestDll mempool_src CppUTest CppUTestExt)

add_executable(TestDllSanityCheck TestRunner.cpp TestDllSanityCheck.cpp)
target_link_libraries(TestDllSanityCheck mempool_src_sanity_check CppUTest CppUTestExt)

add_executable(TestBit TestRunner.cpp TestBit.cpp)
target_link_libraries(TestBit CppUTest CppUTestExt)

add_executable(TestMempool TestRunner.cpp TestMempool.cpp)
target_link_libraries(TestMempool mempool_src CppUTest CppUTestExt)

add_executable(TestMempoolSanityCheck TestRunner.cpp TestMempoolSanityCheck.cpp)
target_link_libraries(TestMempoolSanityCheck mempool_src_sanity_check CppUTest CppUTestExt)

# Test suites
add_test(NAME TestDll COMMAND TestDll -v)
add_test(NAME TestDllSanityCheck COMMAND TestDllSanityCheck -v)
add_test(NAME TestBit COMMAND TestBit -v)
add_test(NAME TestMempool COMMAND TestMempool -v)
add_test(NAME TestMempoolSanityCheck COMMAND TestMempoolSanityCheck -v)