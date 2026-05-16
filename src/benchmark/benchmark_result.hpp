#pragma once

#include <vector>

struct BenchmarkResult {
    float cpu_batch_mean_time_ms;
    float gpu_batch_mean_time_ms;
    float cpu_batch_std_time_ms;
    float gpu_batch_std_time_ms;
    float cpu_time_per_option_price_ms;
    float gpu_time_per_option_price_ms;
    float avg_gpu_speedup; // ratio of gpu speed to cpu speed
    
    std::vector<float> cpu_times_ms;
    std::vector<float> gpu_times_ms;

    size_t num_options;
    int steps;
};