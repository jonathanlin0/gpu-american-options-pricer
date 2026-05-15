#pragma once

enum class OptionType {
    Call,
    Put
};

struct Option {
    double spot;
    double strike;
    double maturity_years; // in years
    double risk_free_rate_annualized; // annualized risk-free rate
    double vol_annualized; // annualized volatility
    OptionType type;
};