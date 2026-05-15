# GPU-Accelerated Fair Value Options Pricing Engine

## Installation and Usage

## Project Description

This project will price the fair value of American options using the binomial model.

### Technical Challenges

American options can be exercised at any moment before the expiration date, while European options can only be exercised at the expiry. Thus, we're unable to use the Black-Scholes model to easily calculate fair values. Instead, we must use a binomial model to recursively price the original option. This lattice structure can be parallelized on a GPU for performance improvements.

For a single equity option, the cartesian product of expiration dates and strike prices is extremely large. Thus, the high parallelism of GPUs can accelerate fair-value calculations across an entire option chain and across different chains much faster than CPU-based programs.

## Project Features

- GPU-based fair value options pricer
- CPU-based fair value options pricer
- test cases ensuring cpu and gpu correctness in `src/tests/`

## Results

### Performance Analysis (CPU vs GPU)

TODO: figure that shows effect of granularity in binomial model on performance
TODO: figure that shows effect of number of different option chains on performance

## Potential Improvements
