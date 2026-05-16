#pragma once

#include <vector>
#include "benchmark_result.hpp"
#include "../pricing/option.hpp"
#include "../pricing/cpu_pricer.hpp"


class BenchmarkRunner {
public:
    BenchmarkRunner(int warmup_runs, int measured_runs, CpuPricer cpu_pricer);

    BenchmarkResult run(
        const std::vector<Option>& options,
        int steps
    ) const;

private:
    int warmup_runs_;
    int measured_runs_;
    CpuPricer cpu_pricer_;
};

