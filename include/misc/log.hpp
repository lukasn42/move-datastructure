#pragma once

std::string time_diff(std::chrono::steady_clock::time_point time);

std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time);

void log_memory_usage(size_t baseline, std::string message);