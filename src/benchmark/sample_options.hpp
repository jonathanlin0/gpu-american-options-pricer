#pragma once

#include <string>
#include <vector>

#include "../pricing/option.hpp"

/**
 * Generates synthetic option data for benchmarking.
 *
 * @param size Benchmark workload size. Must be "small", "medium", or "large".
 * @return A vector of generated options.
 * @throws invalid_argument if size is not one of the supported values.
 */
std::vector<Option> make_sample_option_chain(const std::string& size);