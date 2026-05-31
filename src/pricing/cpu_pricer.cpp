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
        double delta_t = static_cast<double>(option.maturity_years) / steps;
        double u = std::exp(static_cast<double>(option.vol_annualized) * std::sqrt(delta_t));
        double d = std::exp(-static_cast<double>(option.vol_annualized) * std::sqrt(delta_t));
        double r = option.risk_free_rate_annualized;
        double q = (std::exp(r * delta_t) - d) / (u - d); // risk-neutral chance of stock going up

        // fill out payoffs for end state at time option.maturity_years
        // payoffs[i] is payoff with i net "ups"
        vector<double> payoffs(steps + 1);
        for (int i = 0; i < steps + 1; i++) {
            double new_price = std::pow(u, i) * std::pow(d, steps - i) * option.spot;
            if (option.option_type == OptionType::Call) {
                payoffs[i] = std::fmax(new_price - option.strike, 0.0);
            } else {
                payoffs[i] = std::fmax(option.strike - new_price, 0.0);
            }
        }

        // backward induction
        for (int t = steps - 1; t >= 0; t--) {
            for (int i = 0; i <= t; i++) {
                double current_spot = option.spot * std::pow(u, i) * std::pow(d, t-i);

                // value if the option is exercised now
                double curr_exercise_value;
                if (option.option_type == OptionType::Call) {
                    curr_exercise_value = std::fmax(current_spot - option.strike, 0.0);
                } else {
                    curr_exercise_value = std::fmax(option.strike - current_spot, 0.0);
                }

                // EV of next step, discounted by risk free rate r
                double continuation_value = std::exp(-r * delta_t) * (q * payoffs[i+1] + (1-q) * payoffs[i]);

                payoffs[i] = std::fmax(
                    curr_exercise_value,
                    continuation_value
                );
            }
        }

        return static_cast<float>(payoffs[0]);
    }
}


vector<float> CpuPricer::price(
    const vector<Option>& options,
    int steps
) const {
    if (steps <= 0) {
        throw std::invalid_argument("steps must be positive");
    }

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
