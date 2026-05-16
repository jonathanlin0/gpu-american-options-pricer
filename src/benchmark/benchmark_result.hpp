#pragma once

#include <vector>

struct BenchmarkResult {
    float avg_cpu_time_ms;
    float avg_gpu_time_ms;
    float std_cpu_time_ms;
    float std_gpu_time_ms;
    float p99_cpu_time_ms;
    float p99_gpu_time_ms;
    float avg_gpu_speedup; // ratio of gpu speed to cpu speed
    float p99_gpu_speedup;

    std::vector<float> cpu_times_ms;
    std::vector<float> gpu_times_ms;
    std::vector<float> cpu_prices;
    std::vector<float> gpu_prices;

    int num_options;
    int steps;
};