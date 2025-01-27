cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(sleepiDOX VERSION 0.1)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(sleepiDOX ${SRC_FILES})

target_include_directories(sleepiDOX PUBLIC ${CMAKE_CURRECT_SOURCE_DIR}/src/Headers)