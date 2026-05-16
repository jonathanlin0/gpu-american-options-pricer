cmake_minimum_required(VERSION 3.20)

project(gpu_american_options_pricer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(options_pricer
    src/main.cpp
    src/benchmark/benchmark_runner.cpp
    src/benchmark/sample_options.cpp
    src/pricing/cpu_pricer.cpp
)

target_include_directories(options_pricer PRIVATE src)

# some flags for useful compiler warnings
target_compile_options(options_pricer PRIVATE
    -Wall # 
    -Wextra
    -Wpedantic
)