cmake_minimum_required(VERSION "3.16.3")
set(CMAKE_CXX_STANDARD 20)

project(mds CXX)

# openmp
find_package(OpenMP)

# ips4o
add_subdirectory(extern/ips4o)

# concurrentqueue
add_subdirectory(extern/concurrentqueue)

# malloc_count
include_directories(extern/malloc_count)

# libsais
include_directories(extern/libsais/src)

# move datastructure
add_library(mds STATIC src/mds.cpp include/mds.hpp)
target_link_libraries(mds dl ips4o OpenMP::OpenMP_CXX)

# move datastructure test
add_executable(mds_test test/test.cpp)
target_link_libraries(mds_test mds dl ips4o OpenMP::OpenMP_CXX)
target_compile_options(mds_test PUBLIC -Ofast -DNDEBUG -fstrict-aliasing -funroll-loops -finline-functions -march=native)

add_executable(mds_test_debug test/test.cpp)
target_link_libraries(mds_test_debug mds dl ips4o OpenMP::OpenMP_CXX)
target_compile_options(mds_test_debug PUBLIC -ggdb3 -Wall -Wextra -march=native -fstrict-aliasing)