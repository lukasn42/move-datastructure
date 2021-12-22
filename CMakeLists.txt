cmake_minimum_required(VERSION "3.21.3")

set(This move_datastructure)
project(${This} CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
enable_testing()
add_subdirectory(googletest)
set(Headers
    move_datastructure.h
)
set(Sources
    move_datastructure.cpp
)
add_library(${This} STATIC ${Sources} ${Headers})
add_subdirectory(tests)