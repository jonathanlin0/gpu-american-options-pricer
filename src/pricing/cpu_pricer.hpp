#pragma once

#include <vector>
#include <cstddef>
#include "option.hpp"

class CpuPricer {
public:
    CpuPricer(std::size_t num_threads);

    std::vector<float> price(
        const std::vector<Option>& options,
        int steps
    ) const;

private:
    std::size_t num_threads_;
};
