#pragma once

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
};