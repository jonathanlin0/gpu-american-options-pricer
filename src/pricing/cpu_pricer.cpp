#include "cpu_pricer.hpp"
#include <cmath>
#include <algorithm>

using std::vector;
using std::pow;
using std::exp;
using std::sqrt;
using std::max;

vector<float> CpuPricer::price(
    const vector<Option>& options,
    int steps
) const {

    vector<float> prices;

    for (const Option& option : options) {
        float delta_t = option.maturity_years / steps;
        float u = exp(option.vol_annualized * sqrt(delta_t));
        float d = exp(-option.vol_annualized * sqrt(delta_t));
        float r = option.risk_free_rate_annualized;
        float q = (exp(r * delta_t) - d) / (u - d); // risk-neutral chance of stock going up

        // fill out payoffs for end state at time option.maturity_years
        // payoffs[i] is payoff with i net "ups"
        vector<float> payoffs(steps + 1);
        for (int i = 0; i < steps + 1; i++) {
            float new_price = pow(u, i) * pow(d, steps - i) * option.spot;
            if (option.option_type == OptionType::Call) {
                payoffs[i] = max(new_price - option.strike, 0.0f);
            } else {
                payoffs[i] = max(option.strike - new_price, 0.0f);
            }
        }

        // backward induction
        for (int t = steps - 1; t >= 0; t--) {
            for (int i = 0; i <= t; i++) {
                float current_spot = option.spot * pow(u, i) * pow(d, t-i);

                // value if the option is exercised now
                float curr_exercise_value;
                if (option.option_type == OptionType::Call) {
                    curr_exercise_value = max(current_spot - option.strike, 0.0f);
                } else {
                    curr_exercise_value = max(option.strike - current_spot, 0.0f);
                }

                // EV of next step, discounted by risk free rate r
                float continuation_value = exp(-r * delta_t) * (q * payoffs[i+1] + (1-q) * payoffs[i]);

                payoffs[i] = max(
                    curr_exercise_value,
                    continuation_value
                );
            }
        }

        prices.push_back(payoffs[0]);
    }

    return prices;
}