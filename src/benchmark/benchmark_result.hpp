#pragma once

#include <vector>

struct BenchmarkResult {
    float cpu_mean_time_ms;
    float gpu_mean_time_ms;
    float cpu_std_time_ms;
    float gpu_std_time_ms;
    float avg_gpu_speedup; // ratio of gpu speed to cpu speed

    std::vector<float> cpu_times_ms;
    std::vector<float> gpu_times_ms;

    size_t num_options;
    int steps;
};