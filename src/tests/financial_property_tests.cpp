#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "pricing/cpu_pricer.hpp"
#include "pricing/gpu_pricer.cuh"
#include "pricing/option.hpp"

namespace {

constexpr int PROPERTY_STEPS = 500;
constexpr float EPSILON = 1e-4f;

enum class PricerKind {
    Cpu,
    Gpu,
};

float intrinsic_value(const Option& option) {
    if (option.option_type == OptionType::Call) {
        return std::fmax(option.spot - option.strike, 0.0f);
    }
    return std::fmax(option.strike - option.spot, 0.0f);
}

void expect_non_decreasing(
    const std::vector<float>& values,
    const std::string& property_name
) {
    for (std::size_t i = 1; i < values.size(); i++) {
        EXPECT_GE(values[i] + EPSILON, values[i - 1])
            << property_name
            << " failed at index " << i
            << ": previous=" << values[i - 1]
            << ", current=" << values[i];
    }
}

void expect_non_increasing(
    const std::vector<float>& values,
    const std::string& property_name
) {
    for (std::size_t i = 1; i < values.size(); i++) {
        EXPECT_LE(values[i], values[i - 1] + EPSILON)
            << property_name
            << " failed at index " << i
            << ": previous=" << values[i - 1]
            << ", current=" << values[i];
    }
}

std::string pricer_name(
    const testing::TestParamInfo<PricerKind>& info
) {
    return info.param == PricerKind::Cpu ? "CPU" : "GPU";
}

class FinancialPropertyTest : public testing::TestWithParam<PricerKind> {
protected:
    std::vector<float> price(
        const std::vector<Option>& options,
        int steps
    ) const {
        static const CpuPricer cpu_pricer{4};
        static const GpuPricer gpu_pricer{};

        if (GetParam() == PricerKind::Cpu) {
            return cpu_pricer.price(options, steps);
        }
        return gpu_pricer.price(options, steps);
    }
};

TEST_P(FinancialPropertyTest, CallPriceIncreasesWithSpot) {
    // Calls have positive exposure to the underlying price, so increasing spot
    // should not reduce fair value
    const std::vector<Option> options = {
        {80.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {120.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
    };

    expect_non_decreasing(
        price(options, PROPERTY_STEPS),
        "Call price should increase as spot increases"
    );
}

TEST_P(FinancialPropertyTest, PutPriceDecreasesWithSpot) {
    // Puts have negative exposure to the underlying price, so increasing spot
    // should not increase fair value
    const std::vector<Option> options = {
        {80.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {120.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
    };

    expect_non_increasing(
        price(options, PROPERTY_STEPS),
        "Put price should decrease as spot increases"
    );
}

TEST_P(FinancialPropertyTest, CallPriceDecreasesWithStrike) {
    // A higher strike makes a call harder to finish in the money, so call value
    // should not increase with strike
    const std::vector<Option> options = {
        {100.0f, 80.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 120.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
    };

    expect_non_increasing(
        price(options, PROPERTY_STEPS),
        "Call price should decrease as strike increases"
    );
}

TEST_P(FinancialPropertyTest, PutPriceIncreasesWithStrike) {
    // A higher strike makes a put more valuable because the right to sell is
    // worth more
    const std::vector<Option> options = {
        {100.0f, 80.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 120.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
    };

    expect_non_decreasing(
        price(options, PROPERTY_STEPS),
        "Put price should increase as strike increases"
    );
}

TEST_P(FinancialPropertyTest, PricesAreNonNegativeAndAtLeastIntrinsic) {
    // Option values should never be negative, and American options should be
    // worth at least their immediate exercise value. This catches missing
    // max(exercise, continuation) logic and discounting bugs.
    const std::vector<Option> options = {
        {80.0f, 100.0f, 0.5f, 0.04f, 0.20f, OptionType::Call},
        {120.0f, 100.0f, 0.5f, 0.04f, 0.20f, OptionType::Call},
        {80.0f, 100.0f, 0.5f, 0.04f, 0.20f, OptionType::Put},
        {120.0f, 100.0f, 0.5f, 0.04f, 0.20f, OptionType::Put},
        {100.0f, 100.0f, 2.0f, 0.04f, 0.35f, OptionType::Call},
        {100.0f, 100.0f, 2.0f, 0.04f, 0.35f, OptionType::Put},
    };
    const std::vector<float> prices = price(options, PROPERTY_STEPS);

    ASSERT_EQ(prices.size(), options.size());
    for (std::size_t i = 0; i < options.size(); i++) {
        EXPECT_GE(prices[i], -EPSILON)
            << "Option price should never be negative";
        EXPECT_GE(prices[i] + EPSILON, intrinsic_value(options[i]))
            << "American option price should be at least intrinsic value";
    }
}

TEST_P(FinancialPropertyTest, PricesIncreaseWithVolatility) {
    // Higher volatility should increase option value because payoff is convex
    // in the underlying. This catches incorrect u/d multipliers, risk-neutral
    // probability calculations, or volatility units.
    const std::vector<Option> call_options = {
        {100.0f, 100.0f, 1.0f, 0.04f, 0.10f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.40f, OptionType::Call},
    };
    const std::vector<Option> put_options = {
        {100.0f, 100.0f, 1.0f, 0.04f, 0.10f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.40f, OptionType::Put},
    };

    expect_non_decreasing(
        price(call_options, PROPERTY_STEPS),
        "Call price should increase as volatility increases"
    );
    expect_non_decreasing(
        price(put_options, PROPERTY_STEPS),
        "Put price should increase as volatility increases"
    );
}

TEST_P(FinancialPropertyTest, PricesIncreaseWithMaturity) {
    // With the fixed inputs used here, more time should not reduce the value of
    // the American option because the holder has at least the shorter exercise
    // opportunities plus additional optionality.
    const std::vector<Option> call_options = {
        {100.0f, 100.0f, 0.25f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Call},
        {100.0f, 100.0f, 2.0f, 0.04f, 0.25f, OptionType::Call},
    };
    const std::vector<Option> put_options = {
        {100.0f, 100.0f, 0.25f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.25f, OptionType::Put},
        {100.0f, 100.0f, 2.0f, 0.04f, 0.25f, OptionType::Put},
    };

    expect_non_decreasing(
        price(call_options, PROPERTY_STEPS),
        "Call price should increase as maturity increases"
    );
    expect_non_decreasing(
        price(put_options, PROPERTY_STEPS),
        "Put price should increase as maturity increases"
    );
}

INSTANTIATE_TEST_SUITE_P(
    Pricers,
    FinancialPropertyTest,
    testing::Values(PricerKind::Cpu, PricerKind::Gpu),
    pricer_name
);

} // namespace
