#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <thread>

#include "benchmark/sample_options.hpp"
#include "benchmark/benchmark_runner.hpp"
#include "pricing/cpu_pricer.hpp"

int main(int argc, char* argv[]) {
    std::size_t num_workers = 8;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--num_workers" && i + 1 < argc) {
            try {
                num_workers = std::stoul(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num_workers must be an integer\n";
                return 1;
            } 
        } else {
            std::cerr << "Usage: " << argv[0] << " [--num_workers <N>]" << std::endl;
            return 1;
        }
    }

    // cap num_workers to the number of logical cores on the cpu
    try {
        unsigned int num_logical_cores = std::thread::hardware_concurrency(); // returns 0 if unable to detect
        if (num_logical_cores != 0) {
            num_workers = std::min(static_cast<std::size_t>(num_logical_cores), num_workers);
        }
    } catch (const std::exception&) {
        std::cout << "Failed to retrieve maximum number of logical cores." << std::endl;
    }

    std::cout << "num_workers: " << num_workers << std::endl;
    


    // get option chains
    std::vector<Option> options = make_sample_option_chain("small");

    int steps = 1000;
    int warmup_runs = 3;
    int measured_runs = 10;

    CpuPricer cpu_pricer{ num_workers };
    GpuPricer gpu_pricer;

    BenchmarkRunner benchmark_runner(
        warmup_runs,
        measured_runs,
        cpu_pricer,
        gpu_pricer
    );

    // run experiments
    BenchmarkResult result = benchmark_runner.run(options, steps);

    std::cout << "Number of options per step: " << result.num_options << std::endl;
    std::cout << "Steps: " << result.steps << std::endl;
    std::cout << "Speedup: gpu is " << result.avg_gpu_speedup << "x speed of cpu" << std::endl;
    std::cout << std::endl;
    std::cout << "CPU mean time per option (ms): " << result.cpu_time_per_option_price_ms << std::endl;
    std::cout << "CPU step mean time (ms): " << result.cpu_batch_mean_time_ms << std::endl;
    std::cout << "CPU step std dev (ms): " << result.cpu_batch_std_time_ms << std::endl;
    std::cout << std::endl;
    std::cout << "GPU mean time per option (ms): " << result.gpu_time_per_option_price_ms << std::endl;
    std::cout << "GPU step mean time (ms): " << result.gpu_batch_mean_time_ms << std::endl;
    std::cout << "GPU step std dev (ms): " << result.gpu_batch_std_time_ms << std::endl;
    

    // possibly save benchmark result for python plotting

    return 0;
}