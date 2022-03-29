cmake_minimum_required(VERSION "3.16.3")
set(CMAKE_CXX_STANDARD 20)

project(mds LANGUAGES C CXX)

find_package(TBB REQUIRED)

# openmp
find_package(OpenMP)

# ips4o
add_subdirectory(extern/ips4o)

# concurrentqueue
include_directories(extern/concurrentqueue)
add_subdirectory(extern/concurrentqueue)

# malloc_count
include_directories(extern/malloc_count)
add_library(malloc_count STATIC extern/malloc_count/malloc_count.c)

# libsais
include_directories(extern/libsais/src)
add_library(libsais STATIC extern/libsais/src/libsais.c extern/libsais/src/libsais64.c)
target_link_libraries(libsais dl OpenMP::OpenMP_C)

# move datastructure
include_directories(
    src/ src/mdsb/ src/misc/ test/
    include/ include/mdsb/ include/misc/
)
add_library(mds STATIC src/mds.cpp src/mdsb/mdsb.cpp src/misc/avl_tree.cpp src/misc/dl_list.cpp src/misc/dg_io_nc.cpp src/misc/log.cpp)
target_link_libraries(mds dl OpenMP::OpenMP_CXX TBB::tbb ips4o malloc_count)

# move datastructure test
add_executable(mds_test test/test.cpp)
target_link_libraries(mds_test dl OpenMP::OpenMP_CXX TBB::tbb ips4o malloc_count libsais mds)
target_compile_options(mds_test PUBLIC -L$TBBROOT/lib/intel64/gcc4.8 -ltbbmalloc_proxy -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -Ofast -DNDEBUG -fstrict-aliasing -ftree-vectorize -funroll-loops -finline-functions -march=native)

add_executable(mds_test_debug test/test.cpp)
target_link_libraries(mds_test_debug dl OpenMP::OpenMP_CXX TBB::tbb ips4o malloc_count libsais mds)
target_compile_options(mds_test_debug PUBLIC -ggdb3 -Wall -Wextra -march=native -fstrict-aliasing)