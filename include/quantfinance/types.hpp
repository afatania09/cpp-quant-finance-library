#pragma once

#include <stdexcept>

namespace qf {

enum class OptionType { Call, Put };
enum class ExerciseStyle { European, American };

struct Option {
    double spot{};
    double strike{};
    double rate{};
    double dividend_yield{};
    double volatility{};
    double maturity{};
    OptionType type{OptionType::Call};

    void validate() const {
        if (spot <= 0.0 || strike <= 0.0) {
            throw std::invalid_argument("Spot and strike must be positive");
        }
        if (volatility < 0.0 || maturity < 0.0) {
            throw std::invalid_argument("Volatility and maturity cannot be negative");
        }
    }
};

struct Greeks {
    double delta{};
    double gamma{};
    double vega{};
    double theta{};
    double rho{};
};

}  // namespace qf

