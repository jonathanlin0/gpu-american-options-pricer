#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "benchmark/sample_options.hpp"
#include "benchmark/benchmark_runner.hpp"
#include "pricing/cpu_pricer.hpp"

int main(int argc, char* argv[]) {
    std::size_t num_workers = 8;
    int steps = 1000;
    std::string output_dir = "data/";
    std::string size = "small";
    std::size_t num_options = 0; // dummy value. will be populated by command line argument if used. will be casted to size_t in helper function

    bool provided_size = false;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--num-workers" && i + 1 < argc) {
            try {
                num_workers = std::stoul(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num-workers must be a positive integer\n";
                return 1;
            }
        } else if (std::string(argv[i]) == "--steps" && i + 1 < argc) {
            try {
                steps = std::stoi(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--steps must be an integer\n";
                return 1;
            }
        } else if (std::string(argv[i]) == "--size" && i + 1 < argc) {
            size = argv[++i];
            provided_size = true;
        } else if (std::string(argv[i]) == "--num-options" && i + 1 < argc) {
            try {
                num_options = std::stoul(argv[++i]);
                assert(num_options > 0);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num-options must be a positive integer\n";
                return 1;
            }
        } else if (std::string(argv[i]) == "--output-dir" && i + 1 < argc) {
            output_dir = argv[++i];
        } else {
            std::cerr << "Usage: " << argv[0] << " [--num-workers <N>] [--steps <N>] [--size <small|medium|large>] [--output-dir <PATH>]" << std::endl;
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
    std::cout << "steps: " << steps << std::endl;
    std::cout << "output_dir: " << output_dir << std::endl;
    std::cout << "size: " << size << std::endl;

    // create output dir if it doesn't exist yet
    std::filesystem::create_directories(output_dir);

    // create logging jsonl file if it doesn't exist yet
    // TODO: change output_dir to just output_file
    std::string jsonl_filepath = output_dir
        + "data.jsonl";
    {
        std::ofstream jsonl_file(jsonl_filepath, std::ios::app);
        if (!jsonl_file) {
            std::cerr << "Failed to create or open file: " << jsonl_filepath << std::endl;
            return 1;
        }
    }
    


    // get option chains
    std::vector<Option> options = provided_size ? make_sample_option_chain(size) : make_sample_option_chain(num_options);

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
    

    // save benchmark result for python plotting
    std::ofstream jsonl_file(jsonl_filepath, std::ios::app);
    if (!jsonl_file) {
        std::cerr << "Failed to open file for appending: " << jsonl_filepath << std::endl;
        return 1;
    }

    auto write_float_array = [](const std::vector<float>& values) {
        std::ostringstream array_stream;
        array_stream << std::setprecision(9);
        array_stream << "[";
        for (std::size_t i = 0; i < values.size(); i++) {
            if (i > 0) {
                array_stream << ",";
            }
            array_stream << values[i];
        }
        array_stream << "]";
        return array_stream.str();
    };

    jsonl_file << std::setprecision(9);
    jsonl_file
        << "{"
        << "\"num_workers\":" << num_workers << ","
        << "\"steps\":" << result.steps << ","
        << "\"size\":\"" << size << "\","
        << "\"warmup_runs\":" << warmup_runs << ","
        << "\"measured_runs\":" << measured_runs << ","
        << "\"num_options\":" << result.num_options << ","
        << "\"cpu_batch_mean_time_ms\":" << result.cpu_batch_mean_time_ms << ","
        << "\"gpu_batch_mean_time_ms\":" << result.gpu_batch_mean_time_ms << ","
        << "\"cpu_batch_std_time_ms\":" << result.cpu_batch_std_time_ms << ","
        << "\"gpu_batch_std_time_ms\":" << result.gpu_batch_std_time_ms << ","
        << "\"cpu_time_per_option_price_ms\":" << result.cpu_time_per_option_price_ms << ","
        << "\"gpu_time_per_option_price_ms\":" << result.gpu_time_per_option_price_ms << ","
        << "\"avg_gpu_speedup\":" << result.avg_gpu_speedup << ","
        << "\"cpu_times_ms\":" << write_float_array(result.cpu_times_ms) << ","
        << "\"gpu_times_ms\":" << write_float_array(result.gpu_times_ms)
        << "}" << std::endl;
    jsonl_file.close();

    return 0;
}
