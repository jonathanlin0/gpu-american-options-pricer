#pragma once

#include <string>
#include <vector>

#include "../pricing/option.hpp"

std::vector<Option> make_sample_option_chain(const std::string& size);