cmake_minimum_required(VERSION 3.20)

project(gpu_american_options_pricer LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES native)

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(options_pricer
    src/main.cpp
    src/benchmark/benchmark_runner.cpp
    src/benchmark/sample_options.cpp
    src/pricing/cpu_pricer.cpp
    src/pricing/gpu_pricer.cu
)

target_include_directories(options_pricer PRIVATE src)

add_executable(call_surface
    src/examples/call_surface.cpp
    src/pricing/gpu_pricer.cu
)

target_include_directories(call_surface PRIVATE src)

add_executable(pricer_tests
    src/tests/cpu_gpu_consistency_tests.cpp
    src/tests/financial_property_tests.cpp
    src/pricing/cpu_pricer.cpp
    src/pricing/gpu_pricer.cu
)

target_include_directories(pricer_tests PRIVATE src)
target_link_libraries(pricer_tests PRIVATE GTest::gtest_main)

gtest_discover_tests(pricer_tests)

# some flags for useful compiler warnings
target_compile_options(options_pricer PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
)

target_compile_options(call_surface PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
)

target_compile_options(pricer_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>
)
