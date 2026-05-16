#pragma once

#include <stdexcept>

enum class OptionType {
    Call,
    Put
};

struct Option {
    float spot;
    float strike;
    float maturity_years; // in years
    float risk_free_rate_annualized; // annualized risk-free rate
    float vol_annualized; // annualized volatility
    OptionType option_type;

    Option(
        float spot,
        float strike,
        float maturity_years,
        float risk_free_rate_annualized,
        float vol_annualized,
        OptionType option_type
    ): 
        spot(spot),
        strike(strike),
        maturity_years(maturity_years),
        risk_free_rate_annualized(risk_free_rate_annualized),
        vol_annualized(vol_annualized),
        option_type(option_type)
    {
        if (maturity_years <= 0.0f) {
            throw std::invalid_argument("maturity_years must be positive");
        }

        if (vol_annualized <= 0.0f) { // we don't consider 0 vol options, or else that's the trivial case and no point in solving for the value
            throw std::invalid_argument("vol_annualized must be positive");
        }
    }

};