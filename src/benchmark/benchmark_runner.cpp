
#include "benchmark_runner.hpp"
#include <chrono>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <cassert>


using std::accumulate;
using std::sqrt;
using std::vector;
using std::cout;
using std::endl;

constexpr float ABSOLUTE_TOLERANCE = 1e-4f;
constexpr float RELATIVE_TOLERANCE = 1e-5f;

BenchmarkRunner::BenchmarkRunner(
    int warmup_runs,
    int measured_runs,
    CpuPricer cpu_pricer,
    GpuPricer gpu_pricer
)
    : warmup_runs_(warmup_runs),
      measured_runs_(measured_runs),
      cpu_pricer_(cpu_pricer),
      gpu_pricer_(gpu_pricer)
{
    if (warmup_runs < 0) {
        throw std::invalid_argument("warmup_runs must be non-negative");
    }
    if (measured_runs <= 0) {
        throw std::invalid_argument("measured_runs must be positive");
    }
}

BenchmarkResult BenchmarkRunner::run(
    const std::vector<Option>& options,
    int steps
) const {
    if (options.empty()) {
        throw std::invalid_argument("options must not be empty");
    }
    if (steps <= 0) {
        throw std::invalid_argument("steps must be positive");
    }
    
    // warmup
    cout << "Starting warmup..." << endl;
    for (int i = 0; i < warmup_runs_; i++) {
        cpu_pricer_.price(options, steps); // cpu warmup isn't rly necessary. maybe remove in future
        gpu_pricer_.price(options, steps);
    }
    cout << "Finished warmup" << endl;

    // timed runs
    // TODO: maybe in future run all the cpu batches first, then run all the gpu batches for more realistic steady state perf
    vector<float> cpu_times_ms;
    vector<float> gpu_times_ms;
    cout << "Starting timed batch runs..." << endl;
    for (int i = 0; i < measured_runs_; i++) {
        cout << "batch " << i << "/" << measured_runs_ << endl;

        auto cpu_start = std::chrono::steady_clock::now();
        vector<float> cpu_prices = cpu_pricer_.price(options, steps); // can maybe save the prices in the future to check error margin
        auto cpu_end = std::chrono::steady_clock::now();
        float cpu_elapsed_ms = std::chrono::duration<float, std::milli>(cpu_end - cpu_start).count();

        auto gpu_start = std::chrono::steady_clock::now();
        vector<float> gpu_prices = gpu_pricer_.price(options, steps); // can maybe save the prices in the future to check error margin
        auto gpu_end = std::chrono::steady_clock::now();
        float gpu_elapsed_ms = std::chrono::duration<float, std::milli>(gpu_end - gpu_start).count();

        for (size_t j = 0; j < options.size(); j++) {
            if (abs(cpu_prices[j] - gpu_prices[j]) > ABSOLUTE_TOLERANCE + (RELATIVE_TOLERANCE * abs(cpu_prices[j]))) {
                cout << "cpu price " << cpu_prices[j] << " and gpu price " << gpu_prices[j] << " diverged." << endl;
                throw std::runtime_error("CPU and GPU prices diverged");
            }
        }

        cpu_times_ms.push_back(cpu_elapsed_ms);
        gpu_times_ms.push_back(gpu_elapsed_ms);
    }
    cout << "Finished timed batch runs" << endl;

    float cpu_batch_mean_time_ms = accumulate(cpu_times_ms.begin(), cpu_times_ms.end(), 0.0f) / cpu_times_ms.size();
    float cpu_batch_std_time_ms = sqrt(accumulate(
        cpu_times_ms.begin(),
        cpu_times_ms.end(),
        0.0f,
        [cpu_batch_mean_time_ms](float acc, float x) {
            return acc + (x - cpu_batch_mean_time_ms) * (x - cpu_batch_mean_time_ms);
        }
    ) / cpu_times_ms.size());
    float cpu_time_per_option_price_ms = cpu_batch_mean_time_ms / options.size();

    float gpu_batch_mean_time_ms = accumulate(gpu_times_ms.begin(), gpu_times_ms.end(), 0.0f) / gpu_times_ms.size();
    float gpu_batch_std_time_ms = sqrt(accumulate(
        gpu_times_ms.begin(),
        gpu_times_ms.end(),
        0.0f,
        [gpu_batch_mean_time_ms](float acc, float x) {
            return acc + (x - gpu_batch_mean_time_ms) * (x - gpu_batch_mean_time_ms);
        }
    ) / gpu_times_ms.size());
    float gpu_time_per_option_price_ms = gpu_batch_mean_time_ms / options.size();

    return BenchmarkResult{
        cpu_batch_mean_time_ms,
        gpu_batch_mean_time_ms,
        cpu_batch_std_time_ms,
        gpu_batch_std_time_ms,
        cpu_time_per_option_price_ms,
        gpu_time_per_option_price_ms,
        cpu_batch_mean_time_ms / gpu_batch_mean_time_ms,
        cpu_times_ms,
        gpu_times_ms,
        options.size(),
        steps,
    };
}