#pragma once

template <typename T>
std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time);

template <typename T>
void log_memory_usage(size_t baseline, std::string message);