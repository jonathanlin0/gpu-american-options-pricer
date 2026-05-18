#pragma once

#include <string>
#include <vector>

#include "pricing/option.hpp"

/**
 * Generates synthetic option data for benchmarking.
 *
 * @param size Number of options to generate.
 * @return A vector of generated options.
 * @throws invalid_argument if size is not one of the supported values.
 */
std::vector<Option> make_sample_option_chain(const std::size_t num_options);

/**
 * Generates synthetic option data for benchmarking. Small, medium, and large are the predefined sizes for the number
 * of options to generate. In the implementation, it simply calls the make_sample_option_chain(size_t) function with the
 * predefined number for the size.
 *
 * @param size Benchmark workload size. Must be "small", "medium", "large", or a positive number (as a string)
 * @return A vector of generated options.
 * @throws invalid_argument if size is not one of the supported values.
 */
std::vector<Option> make_sample_option_chain(const std::string& size);

