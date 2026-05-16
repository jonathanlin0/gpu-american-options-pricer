#include "gpu_pricer.cuh"
#include "error_check.cuh"
#include <stdexcept>
#include <cmath>
#include <algorithm>

using std::vector;
using std::pow;
using std::exp;
using std::sqrt;
using std::max;

// CUDA launch hyperparameters
constexpr int THREADS_PER_BLOCK = 256;

__global__
void price_kernel(
    const Option* options,
    float* prices,
    int steps
) {
    /*
     * one block is launched per option
     * the threads first calculate the terminal payoffs. each thread calculates a single terminal payoff, and strides by blockDim.x if blockDim.x < steps + 1
     * then, step steps are done to get the final price
     * all this is done in shared memory. optimal cause threads only care about data within the block, and not across blocks.
     *
     * the cpu method modified payoffs in place. we can't do that cause we have no guarantee of warp executation order. thus, we'll use
     * ping-pong buffering. two buffers - old_payoffs and new_payoffs. this'll prevent the likely data corruption that'll come
     * with threads modifying values in place that other threads in other warps may be dependent on.
    */
    const Option option = options[blockIdx.x];

    // have to do this stuff cause cuda only gives 1 shared memory buffer for dynamically allocated shared memory
    extern __shared__ float shared_payoffs[];
    // will be size `steps + 1`
    float* old_payoffs = shared_payoffs;
    float* new_payoffs = shared_payoffs + (steps + 1);

    float delta_t = option.maturity_years / steps;
    float u = expf(option.vol_annualized * sqrtf(delta_t));
    float d = expf(-option.vol_annualized * sqrtf(delta_t));
    float r = option.risk_free_rate_annualized;
    float q = (expf(r * delta_t) - d) / (u - d); // risk-neutral chance of stock going up

    // create initial payoffs
    for (int i = threadIdx.x; i < steps + 1; i += blockDim.x) {
        float new_price = powf(u, i) * powf(d, steps - i) * option.spot;
        if (option.option_type == OptionType::Call) {
            old_payoffs[i] = fmaxf(new_price - option.strike, 0.0f);
        } else {
            old_payoffs[i] = fmaxf(option.strike - new_price, 0.0f);
        }
    }
    __syncthreads();

    // backward induction
    for (int t = steps - 1; t >= 0; t--) {
        for (int i = threadIdx.x; i <= t; i += blockDim.x) {
            float current_spot = option.spot * powf(u, i) * powf(d, t-i);

            // value if the option is exercised now
            float curr_exercise_value;
            if (option.option_type == OptionType::Call) {
                curr_exercise_value = fmaxf(current_spot - option.strike, 0.0f);
            } else {
                curr_exercise_value = fmaxf(option.strike - current_spot, 0.0f);
            }

            // EV of next step, discounted by risk free rate r
            float continuation_value = expf(-r * delta_t) * (q * old_payoffs[i+1] + (1-q) * old_payoffs[i]);

            new_payoffs[i] = fmaxf(
                curr_exercise_value,
                continuation_value
            );
        }

        __syncthreads();
        // swap which one is "old" and which one is "new"
        // old_payoffs <- new_data
        // new_data will now be used as the scratchpad for the next step
        // note: each thread does this, but it's fine cause overhead for assigning pointers is extremely small
        // other option would be to have some flag to flip flop which half of the buffer is new and which is old. but this
        // can get extremely complicated and confusing quickly, so we'll stick w this
        float* temp = old_payoffs;
        old_payoffs = new_payoffs;
        new_payoffs = temp;
        __syncthreads();
    }

    // one thread writes the final answer
    if (threadIdx.x == 0) {
        prices[blockIdx.x] = old_payoffs[0];
    }
    
}

vector<float> GpuPricer::price(
    const vector<Option>& options,
    int steps
) const {
    // we'll assume a max shared memory size per block of 48 KB. you can get more, but then that's architecture specific. so we'll just
    // be safe and stick w 48 KB
    // 2 * (steps + 1) * 4 bytes <= 48 KB = 48000 B
    // steps <= 5999
    // ref: https://docs.nvidia.com/cuda/cuda-programming-guide/03-advanced/advanced-kernel-programming.html
    if (steps > 5999) {
        throw std::invalid_argument("steps must be less than 5999");
    }

    // allocate and fill data for gpu
    float* gpu_prices;
    checkCuda(cudaMalloc(&gpu_prices, sizeof(float) * options.size()));

    Option* gpu_options;
    checkCuda(cudaMalloc(&gpu_options, sizeof(Option) * options.size()));
    checkCuda(cudaMemcpy(gpu_options, options.data(), sizeof(Option) * options.size(), cudaMemcpyHostToDevice));

    size_t shared_mem_bytes = 2 * (steps + 1) * sizeof(float);
    size_t num_blocks = options.size();
    price_kernel<<<num_blocks, THREADS_PER_BLOCK, shared_mem_bytes>>>(
        gpu_options,
        gpu_prices,
        steps
    );
    checkCuda(cudaGetLastError()); // for the kernel launch

    // copy gpu data back to cpu
    vector<float> prices(options.size());
    checkCuda(cudaMemcpy(prices.data(), gpu_prices, sizeof(float) * options.size(), cudaMemcpyDeviceToHost));

    // free gpu memory
    checkCuda(cudaFree(gpu_options));
    checkCuda(cudaFree(gpu_prices));

    return prices;
}
