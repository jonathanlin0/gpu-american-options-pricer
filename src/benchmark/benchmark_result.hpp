#pragma once

#include <vector>

struct BenchmarkResult {
    double avg_cpu_time_ms;
    double avg_gpu_time_ms;
    double p99_cpu_time_ms;
    double p99_gpu_time_ms;
    double avg_gpu_speedup; // ratio of gpu speed to cpu speed
    double p99_gpu_speedup;

    std::vector<double> cpu_times_ms;
    std::vector<double> gpu_times_ms;
    std::vector<double> cpu_prices;
    std::vector<double> gpu_prices;

    int num_options;
    int steps;
};