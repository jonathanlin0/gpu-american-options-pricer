#include "sample_options.hpp"

#include <random>
#include <stdexcept>
#include <unordered_map>

namespace {
    struct SampleOptionChainConfig {
        std::size_t num_option_chains;
    };

    const std::unordered_map<std::string, SampleOptionChainConfig> SAMPLE_OPTION_CHAIN_CONFIGS = {
        { "small", { 1000 } },
        { "medium", { 10000 } },
        { "large", { 100000 } },
    };
}

std::vector<Option> make_sample_option_chain(const std::size_t num_options) {
    if (num_options <= 0) {
        throw std::invalid_argument("num_options must be positive");
    }    
    
    const float risk_free_rate = 0.04;

    std::random_device rd;
    std::mt19937 gen(rd());
    constexpr std::size_t num_expiries_per_chain = 25;
    constexpr std::size_t num_strikes_per_chain = 25;

    std::uniform_real_distribution<float> spot_dist(200.0, 800.0);
    std::uniform_real_distribution<float> strike_multiplier_dist(0.8, 1.2); // as a percentage of the spot. TODO: the range of spot will be adjusted later to reflect 30 delta to 99 delta based on vol for that option chain. for now, it'll just be +- 20% of the spot
    std::uniform_real_distribution<float> maturity_dist(1.0 / 365.0, 2.0);
    std::uniform_real_distribution<float> vol_dist(0.1, 0.35); // 10% to 35%
    std::uniform_real_distribution<float> standard_unif(0.0, 1.0);

    std::vector<Option> options;
    while (options.size() < num_options) {
        float spot = spot_dist(gen);
        float vol = vol_dist(gen);

        for (size_t j = 0; j < num_expiries_per_chain; j++) {
            float maturity = maturity_dist(gen);
            for (size_t k = 0; k < num_strikes_per_chain; k++) {
                float strike = spot * strike_multiplier_dist(gen);
                OptionType option_type = standard_unif(gen) < 0.5
                    ? OptionType::Call
                    : OptionType::Put;

                options.push_back(Option{
                    spot,
                    strike,
                    maturity,
                    risk_free_rate,
                    vol,
                    option_type
                });
            }
        }
    }
    options.erase(options.begin() + num_options, options.end());
    
    return options;
}

std::vector<Option> make_sample_option_chain(const std::string& size) {
    auto config_it = SAMPLE_OPTION_CHAIN_CONFIGS.find(size);
    if (config_it == SAMPLE_OPTION_CHAIN_CONFIGS.end()) {
        throw std::invalid_argument("size must be 'small', 'medium', or 'large'");
    }
    std::size_t num_options = config_it->second.num_option_chains;
    return make_sample_option_chain(num_options);
}