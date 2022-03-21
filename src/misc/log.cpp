#include <iostream>
#include <chrono>

extern "C" {
    #include <malloc_count.h>
}

#include <log.hpp>

std::string time_diff(std::chrono::steady_clock::time_point time) {
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count());
}

std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time) {
    std::cout << ", in ~ " << time_diff(time) << " ms" << std::endl;
    return std::chrono::steady_clock::now();
}

void log_memory_usage(size_t baseline, std::string message) {
    std::cout << message << ": ~ " << (malloc_count_current()-baseline)/1000000 << " MB allocated" << std::flush;
}