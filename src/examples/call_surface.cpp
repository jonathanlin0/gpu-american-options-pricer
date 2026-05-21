// this plots the surface of an option chain wrt the expiry and the strike price

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "pricing/gpu_pricer.cuh"

int main(int argc, char* argv[]) {

    std::string output_dir = "data/";
    std::size_t num_expiries = 100;
    std::size_t num_strikes = 100;
    int num_steps = 500;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--output-dir" && i + 1 < argc) {
            output_dir = argv[++i];
        } else if (std::string(argv[i]) == "--num-expiries" && i + 1 < argc) {
            try {
                num_expiries = std::stoul(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num-expiries must be a positive integer\n";
                return 1;
            }
        } else if (std::string(argv[i]) == "--num-strikes" && i + 1 < argc) {
            try {
                num_strikes = std::stoul(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num-strikes must be a positive integer\n";
                return 1;
            }
        } else if (std::string(argv[i]) == "--num-steps" && i + 1 < argc) {
            try {
                num_steps = std::stoi(argv[++i]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "--num-steps must be a positive integer\n";
                return 1;
            }
        } else {
            std::cerr << "Usage: " << argv[0] << " [--output-dir <PATH>] [--num-expiries <N>] [--num-strikes <N>] [--num-steps <N>]" << std::endl;
            return 1;
        }
    }

    const float risk_free_rate = 0.04;

    constexpr float vol = 0.2;

    float spot = 100;

    constexpr float min_expiry = 1;
    constexpr float max_expiry = 100;
    constexpr float min_strike = 0.8;
    constexpr float max_strike = 1.2;

    std::vector<Option> options;
    for (
        float expiry = min_expiry;
        expiry < max_expiry;
        expiry += (max_expiry - min_expiry) /  num_expiries
    ) {
        for (
            float strike = min_strike;
            strike < max_strike;
            strike += (max_strike - min_strike) /  num_strikes
        ) {
            OptionType option_type = OptionType::Call;

            options.push_back(Option{
                spot,
                strike,
                expiry,
                risk_free_rate,
                vol,
                option_type
            });
        }
    }
    
    GpuPricer gpu_pricer{ };

    std::vector<float> prices = gpu_pricer.price(options, num_steps);

    // create output dir if it doesn't exist yet
    std::filesystem::create_directories(output_dir);

    // write the surface data to a fresh CSV file
    const std::filesystem::path csv_filepath =
        std::filesystem::path(output_dir) / "call_surface.csv";
    std::ofstream csv_file(csv_filepath);
    if (!csv_file) {
        std::cerr << "Failed to create or open file: " << csv_filepath << std::endl;
        return 1;
    }

    csv_file << "price,expiry,strike\n";
    for (std::size_t i = 0; i < prices.size(); i++) {
        csv_file
            << prices[i] << ','
            << options[i].maturity_years << ','
            << options[i].strike << '\n';
    }
    csv_file.close();

}
