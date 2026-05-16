#pragma once

#include <vector>
#include <cstddef>

struct BenchmarkResult {
    float cpu_batch_mean_time_ms;
    float gpu_batch_mean_time_ms;
    float cpu_batch_std_time_ms;
    float gpu_batch_std_time_ms;
    float cpu_time_per_option_price_ms;
    float gpu_time_per_option_price_ms;
    float avg_gpu_speedup; // ratio of cpu speed to gpu speed
    
    std::vector<float> cpu_times_ms;
    std::vector<float> gpu_times_ms;

    std::size_t num_options;
    int steps;
};