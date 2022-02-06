#include <iostream>

extern "C" {
    #include "../../extern/malloc_count/malloc_count.h"
}

#include "../../include/misc/log.hpp"

template <typename T>
std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time) {
    std::cout << ", in ~ " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << " ms" << std::endl;
    return std::chrono::steady_clock::now();
}

template <typename T>
void log_memory_usage(size_t baseline, std::string message) {
    std::cout << message << ": ~ " << (malloc_count_current()-baseline)/1000000 << " MB allocated" << std::flush;
}