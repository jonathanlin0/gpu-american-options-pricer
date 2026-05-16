#include "cpu_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <thread>

using std::vector;
using std::size_t;

CpuPricer::CpuPricer(size_t num_threads) : num_threads_(num_threads) {
    if (num_threads == 0) {
        throw std::invalid_argument("num_threads must be positive");
    }
}

namespace {
    float price_one_option(const Option& option, int steps) {
        float delta_t = option.maturity_years / steps;
        float u = expf(option.vol_annualized * sqrtf(delta_t));
        float d = expf(-option.vol_annualized * sqrtf(delta_t));
        float r = option.risk_free_rate_annualized;
        float q = (expf(r * delta_t) - d) / (u - d); // risk-neutral chance of stock going up

        // fill out payoffs for end state at time option.maturity_years
        // payoffs[i] is payoff with i net "ups"
        vector<float> payoffs(steps + 1);
        for (int i = 0; i < steps + 1; i++) {
            float new_price = powf(u, i) * powf(d, steps - i) * option.spot;
            if (option.option_type == OptionType::Call) {
                payoffs[i] = fmaxf(new_price - option.strike, 0.0f);
            } else {
                payoffs[i] = fmaxf(option.strike - new_price, 0.0f);
            }
        }

        // backward induction
        for (int t = steps - 1; t >= 0; t--) {
            for (int i = 0; i <= t; i++) {
                float current_spot = option.spot * powf(u, i) * powf(d, t-i);

                // value if the option is exercised now
                float curr_exercise_value;
                if (option.option_type == OptionType::Call) {
                    curr_exercise_value = fmaxf(current_spot - option.strike, 0.0f);
                } else {
                    curr_exercise_value = fmaxf(option.strike - current_spot, 0.0f);
                }

                // EV of next step, discounted by risk free rate r
                float continuation_value = expf(-r * delta_t) * (q * payoffs[i+1] + (1-q) * payoffs[i]);

                payoffs[i] = fmaxf(
                    curr_exercise_value,
                    continuation_value
                );
            }
        }

        return payoffs[0];
    }
}


vector<float> CpuPricer::price(
    const vector<Option>& options,
    int steps
) const {

    vector<float> prices(options.size());

    vector<std::thread> threads;
    size_t worker_count = std::min(num_threads_, options.size());

    /*
     * striding pattern. each thread takes care of one option price calculation.
     * will stride option price calculations by `worker_count` if worker_count < options.size()
    */
    for (size_t thread_id = 0; thread_id < worker_count; thread_id++) {
        threads.emplace_back([&, thread_id]() {
            for (size_t i = thread_id; i < options.size(); i += worker_count) {
                prices[i] = price_one_option(options[i], steps);
            }
        });
    }

    for (std::thread& thread : threads) {
        thread.join();
    }


    return prices;
}