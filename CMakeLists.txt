cmake_minimum_required(VERSION 2.6)

project(learn-leveldb CXX)

# build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# c++ standard
set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS
    -g 
    -Wall
    -std=c++11)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/util)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/include/leveldb)
include_directories(${PROJECT_SOURCE_DIR}/src/port)

# test
enable_testing()

add_subdirectory(src/util)
add_subdirectory(src/unittest)
