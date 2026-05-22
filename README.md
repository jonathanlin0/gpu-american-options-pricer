# GPU-Accelerated Fair Value Options Pricing Engine

## Installation and Usage

Required:
- CMake 3.20+
- A C++17 compiler supported by CUDA
- NVIDIA CUDA Toolkit, including `nvcc`
- An NVIDIA GPU with a compatible driver
- A build tool supported by CMake

From the repo root (on Linux computer):
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/options_pricer
```

## Project Description

This project will price the fair value (FV) of American options using the binomial model.

### Technical Challenges

American options can be exercised at any moment before the expiration date, while European options can only be exercised at the expiry. Thus, we're unable to use the Black-Scholes model to easily calculate FVs. Instead, we must use a binomial model to recursively price the original option. This lattice structure can be parallelized on a GPU for performance improvements.

For a single equity option, the cartesian product of expiration dates and strike prices is extremely large. Thus, the high parallelism of GPUs can accelerate FV calculations across an entire option chain and across different chains much faster than CPU-based programs.

## Methodology

### Assumptions
- No dividends
- Lognormal stock-price dynamics: the log price moves up or down by $\pm \sigma \sqrt{\delta t}$

## Project Features

- GPU-based FV options pricer
- CPU-based FV options pricer
- Tests to ensure cpu and gpu correctness in `src/tests/`

## Results

### Performance Analysis (CPU vs GPU)

#### Runtime vs Binomial Steps
![CPU vs GPU batch time by binomial steps](figs/steps_graph.png)

The CPU and GPU runtimes w.r.t. the number of binomial steps. The higher the number of steps, the more accurate the option price estimation is. the CPU runtime is exponential because for each additional layer in the binomial lattice structure, a $t+1$ length array is added. Thus, you must do $t+1$ more calculations (for the final layer payoffs) and $t$ more calculations for the additional layer of backward induction. The GPU runtime remains linear, because all the threads are able to do the initial payoff calculations and the subsequent backward induction calculation in parallel. So for CPU, $O(t)$ runtime is added, while only $O(1)$ runtime is added for GPU. This Figure generated with [`scripts/graph_steps.py`](scripts/graph_steps.py).

#### Runtime vs Number of Options
![CPU vs GPU batch time by number of options](figs/num_options_graph.png)

The CPU and GPU runtimes w.r.t. the number of option to price are both $O(n)$. But the GPU just has a lot better parallelization, so the amount of time required to calculate the price of a single option is a lot lower. This mainly comes from the backward induction GPU speedup.

TODO: figure that shows effect of number of different option chains on performance

## Potential Improvements
- check runtime and accuracy performance from using double instead of floats
- include dividend payments in the pricing engine