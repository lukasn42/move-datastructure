cmake_minimum_required(VERSION "3.16.3")

set(This move_datastructure)
project(${This} CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
enable_testing()
include(FetchContent)

#malloc_count
FetchContent_Declare(
  malloc_count
  URL https://github.com/bingmann/malloc_count/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(malloc_count)

#libsais
FetchContent_Declare(
  libsais
  URL https://github.com/IlyaGrebnov/libsais/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(libsais)

set(Headers
  move_datastructure.h
  dl_list.h
  avl_tree.h
)
set(Sources
  main.cpp
  move_datastructure.cpp
  dl_list.cpp
  avl_tree.cpp
)
add_library(${This} STATIC ${Sources} ${Headers})

# googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)

add_executable(main main.cpp)
target_link_libraries(main dl)