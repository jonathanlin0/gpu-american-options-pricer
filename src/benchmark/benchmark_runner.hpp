#pragma once

#include <vector>
#include "benchmark_result.hpp"
#include "pricing/option.hpp"
#include "pricing/cpu_pricer.hpp"
#include "pricing/gpu_pricer.cuh"


class BenchmarkRunner {
public:
    BenchmarkRunner(int warmup_runs, int measured_runs, CpuPricer cpu_pricer, GpuPricer gpu_pricer);

    BenchmarkResult run(
        const std::vector<Option>& options,
        int steps
    ) const;

private:
    int warmup_runs_;
    int measured_runs_;
    CpuPricer cpu_pricer_;
    GpuPricer gpu_pricer_;
};

