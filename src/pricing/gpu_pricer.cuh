#pragma once

#include <vector>
#include "option.hpp"

class GpuPricer {
public:
    std::vector<float> price(
        const std::vector<Option>& options,
        int steps
    ) const;
};
