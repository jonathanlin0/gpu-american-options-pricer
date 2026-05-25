#include <cmath>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "pricing/cpu_pricer.hpp"
#include "pricing/gpu_pricer.cuh"
#include "pricing/option.hpp"

namespace {

constexpr float ABSOLUTE_TOLERANCE = 1e-3f;
constexpr float RELATIVE_TOLERANCE = 1e-4f;

void expect_cpu_gpu_prices_match(
    const std::vector<Option>& options,
    int steps
) {
    const CpuPricer cpu_pricer{4};
    const GpuPricer gpu_pricer{};

    const std::vector<float> cpu_prices = cpu_pricer.price(options, steps);
    const std::vector<float> gpu_prices = gpu_pricer.price(options, steps);

    ASSERT_EQ(cpu_prices.size(), gpu_prices.size());
    ASSERT_EQ(cpu_prices.size(), options.size());

    for (std::size_t i = 0; i < options.size(); i++) {
        const float tolerance = ABSOLUTE_TOLERANCE +
            RELATIVE_TOLERANCE * std::fabs(cpu_prices[i]);

        EXPECT_NEAR(cpu_prices[i], gpu_prices[i], tolerance)
            << "CPU/GPU price mismatch at option index " << i
            << " with steps=" << steps
            << " cpu price=" << cpu_prices[i]
            << " gpu price=" << gpu_prices[i];
    }
}

TEST(CpuGpuConsistencyTest, SingleCallPricesMatchAcrossStepCounts) {
    // A single call across several lattice depths checks that CPU and GPU
    // backward induction produce the same value from the simplest call path.
    // Small step counts catch boundary cases, while larger counts catch drift.
    const std::vector<Option> options = {
        {100.0f, 105.0f, 1.0f, 0.04f, 0.20f, OptionType::Call},
    };

    for (const int steps : {1, 2, 10, 100, 500, 1500}) {
        expect_cpu_gpu_prices_match(options, steps);
    }
}

TEST(CpuGpuConsistencyTest, SinglePutPricesMatchAcrossStepCounts) {
    // same as above test for calls, but this is for puts
    const std::vector<Option> options = {
        {100.0f, 105.0f, 1.0f, 0.04f, 0.20f, OptionType::Put},
    };

    for (const int steps : {1, 2, 10, 100, 500, 1500}) {
        expect_cpu_gpu_prices_match(options, steps);
    }
}

TEST(CpuGpuConsistencyTest, MixedBatchPricesMatch) {
    // The GPU prices one option per block, so mixed batches check that block
    // indexing, option type handling, and result ordering match the CPU output.
    const std::vector<Option> options = {
        {80.0f, 100.0f, 0.25f, 0.04f, 0.15f, OptionType::Call},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.20f, OptionType::Call},
        {130.0f, 100.0f, 2.0f, 0.04f, 0.35f, OptionType::Call},
        {80.0f, 100.0f, 0.25f, 0.04f, 0.15f, OptionType::Put},
        {100.0f, 100.0f, 1.0f, 0.04f, 0.20f, OptionType::Put},
        {130.0f, 100.0f, 2.0f, 0.04f, 0.35f, OptionType::Put},
    };

    expect_cpu_gpu_prices_match(options, 500);
}

TEST(CpuGpuConsistencyTest, LargeBatchPricesMatch) {
    // A larger generated batch exercises many GPU blocks and a wider range of
    // inputs. This catches indexing, memory-copy, and per-option isolation bugs
    // that may not appear in single-option tests.
    std::vector<Option> options;
    options.reserve(96);

    for (int i = 0; i < 48; i++) {
        // semi-arbitrarily chosen values to ensure a good variety of options
        // ITM, ATM, OTM, high vol, low vol, etc
        const float spot = 75.0f + static_cast<float>(i % 12) * 5.0f;
        const float strike = 70.0f + static_cast<float>(i % 16) * 4.0f;
        const float maturity = 0.25f + static_cast<float>(i % 8) * 0.25f;
        const float vol = 0.12f + static_cast<float>(i % 7) * 0.03f;

        options.push_back({
            spot,
            strike,
            maturity,
            0.04f,
            vol,
            OptionType::Call
        });
        options.push_back({
            spot,
            strike,
            maturity,
            0.04f,
            vol,
            OptionType::Put
        });
    }

    expect_cpu_gpu_prices_match(options, 250);
}

} // namespace
