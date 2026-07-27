#include "quantfinance/implied_volatility.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "quantfinance/black_scholes.hpp"

namespace qf {

double implied_volatility(
    double market_price,
    Option option,
    double tolerance,
    std::size_t max_iterations) {
    option.validate();
    if (market_price < 0.0 || tolerance <= 0.0 || max_iterations == 0) {
        throw std::invalid_argument("Invalid implied-volatility solver input");
    }

    const double discounted_spot =
        option.spot * std::exp(-option.dividend_yield * option.maturity);
    const double discounted_strike =
        option.strike * std::exp(-option.rate * option.maturity);
    const double lower = option.type == OptionType::Call
                             ? std::max(discounted_spot - discounted_strike, 0.0)
                             : std::max(discounted_strike - discounted_spot, 0.0);
    const double upper = option.type == OptionType::Call ? discounted_spot : discounted_strike;
    if (market_price < lower - tolerance || market_price > upper + tolerance) {
        throw std::domain_error("Market price violates no-arbitrage bounds");
    }

    double low_vol = 1e-8;
    double high_vol = 5.0;
    for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
        option.volatility = 0.5 * (low_vol + high_vol);
        const double difference = black_scholes_price(option) - market_price;
        if (std::abs(difference) < tolerance) {
            return option.volatility;
        }
        if (difference > 0.0) {
            high_vol = option.volatility;
        } else {
            low_vol = option.volatility;
        }
    }
    throw std::runtime_error("Implied-volatility solver did not converge");
}

}  // namespace qf

