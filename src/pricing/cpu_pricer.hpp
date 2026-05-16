#pragma once

#include <vector>
#include "option.hpp"

class CpuPricer {
public:
    std::vector<float> price(
        const std::vector<Option>& options,
        int steps
    ) const;
};
