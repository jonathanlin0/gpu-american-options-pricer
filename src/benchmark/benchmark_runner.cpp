
#include "benchmark_runner.hpp"
#include <chrono>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <iostream>


using std::accumulate;
using std::sqrt;
using std::vector;
using std::cout;
using std::endl;

BenchmarkRunner::BenchmarkRunner(
    int warmup_runs,
    int measured_runs,
    CpuPricer cpu_pricer
)
    : warmup_runs_(warmup_runs),
      measured_runs_(measured_runs),
      cpu_pricer_(cpu_pricer)
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
    if (steps <= 0) {
        throw std::invalid_argument("steps must be positive");
    }

    // warmup
    cout << "starting warmup" << endl;
    for (int i = 0; i < warmup_runs_; i++) {
        cpu_pricer_.price(options, steps);
    }

    // timed runs
    vector<float> cpu_times_ms;
    cout << "starting timed runs" << endl;
    for (int i = 0; i < measured_runs_; i++) {
        cout << "run " << i << "/" << measured_runs_ << endl;
        auto start = std::chrono::steady_clock::now();
        cpu_pricer_.price(options, steps); // can maybe save the prices in the future to check error margin
        auto end = std::chrono::steady_clock::now();
        float elapsed_ms = std::chrono::duration<float, std::milli>(end - start).count();

        cpu_times_ms.push_back(elapsed_ms);
    }

    float cpu_mean_time_ms = accumulate(cpu_times_ms.begin(), cpu_times_ms.end(), 0.0f) / cpu_times_ms.size();
    float cpu_std_time_ms = sqrt(accumulate(
        cpu_times_ms.begin(),
        cpu_times_ms.end(),
        0.0f,
        [cpu_mean_time_ms](float acc, float x) {
            return acc + (x - cpu_mean_time_ms) * (x - cpu_mean_time_ms);
        }
    ) / cpu_times_ms.size());

    return BenchmarkResult{
        cpu_mean_time_ms,
        -1,
        cpu_std_time_ms,
        -1,
        -1,
        cpu_times_ms,
        {},
        options.size(),
        steps,
    };
}