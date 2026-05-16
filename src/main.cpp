#include <iostream>
#include <vector>

#include "benchmark/sample_options.hpp"
#include "benchmark/benchmark_runner.hpp"
#include "pricing/cpu_pricer.hpp"

int main() {
    // get option chains
    std::vector<Option> options = make_sample_option_chain("small");

    int steps = 1000;
    int warmup_runs = 3;
    int measured_runs = 10;

    CpuPricer cpu_pricer;

    BenchmarkRunner benchmark_runner(
        warmup_runs,
        measured_runs,
        cpu_pricer
    );

    // run experiments
    BenchmarkResult result = benchmark_runner.run(options, steps);

    std::cout << "Number of options per step: " << result.num_options << '\n';
    std::cout << "Steps: " << result.steps << '\n';
    std::cout << "CPU mean time per option (ms): " << result.cpu_time_per_option_price_ms << '\n';
    std::cout << "CPU step mean time (ms): " << result.cpu_batch_mean_time_ms << '\n';
    std::cout << "CPU step std dev (ms): " << result.cpu_batch_std_time_ms << '\n';
    

    // possibly save benchmark result for python plotting

    return 0;
}