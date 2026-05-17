#include "sample_options.hpp"

#include <random>
#include <stdexcept>
#include <unordered_map>

namespace {
    struct SampleOptionChainConfig {
        int num_option_chains;
        int num_expiries_per_chain;
        int num_strikes_per_chain;
    };

    const std::unordered_map<std::string, SampleOptionChainConfig> SAMPLE_OPTION_CHAIN_CONFIGS = {
        { "small", { 10, 20, 20 } },
        { "medium", { 100, 30, 30 } },
        { "large", { 1000, 50, 50 } },
    };
}

std::vector<Option> make_sample_option_chain(const std::string& size) {
    auto config_it = SAMPLE_OPTION_CHAIN_CONFIGS.find(size);
    if (config_it == SAMPLE_OPTION_CHAIN_CONFIGS.end()) {
        throw std::invalid_argument("size must be 'small', 'medium', or 'large'");
    }
    
    const SampleOptionChainConfig& config = config_it->second;
    const float risk_free_rate = 0.04;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> spot_dist(200.0, 800.0);
    std::uniform_real_distribution<float> strike_multiplier_dist(0.8, 1.2); // as a percentage of the spot. TODO: the range of spot will be adjusted later to reflect 30 delta to 99 delta based on vol for that option chain. for now, it'll just be +- 20% of the spot
    std::uniform_real_distribution<float> maturity_dist(1.0 / 365.0, 2.0);
    std::uniform_real_distribution<float> vol_dist(0.1, 0.35); // 10% to 35%
    std::uniform_real_distribution<float> standard_unif(0.0, 1.0);

    std::vector<Option> options;
    for (int i = 0; i < config.num_option_chains; i++) {
        float spot = spot_dist(gen);
        float vol = vol_dist(gen);

        for (int j = 0; j < config.num_expiries_per_chain; j++) {
            float maturity = maturity_dist(gen);
            for (int k = 0; k < config.num_strikes_per_chain; k++) {
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

    return options;
}
