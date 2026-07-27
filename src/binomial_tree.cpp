#include "quantfinance/binomial_tree.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "quantfinance/black_scholes.hpp"

namespace qf {

double binomial_price(const Option& option, std::size_t steps, ExerciseStyle style) {
    option.validate();
    if (steps == 0) {
        throw std::invalid_argument("Binomial tree requires at least one step");
    }
    if (option.maturity == 0.0) {
        return payoff(option.type, option.spot, option.strike);
    }
    if (option.volatility == 0.0) {
        return black_scholes_price(option);
    }

    const double dt = option.maturity / static_cast<double>(steps);
    const double up = std::exp(option.volatility * std::sqrt(dt));
    const double down = 1.0 / up;
    const double growth = std::exp((option.rate - option.dividend_yield) * dt);
    const double probability = (growth - down) / (up - down);
    if (probability < 0.0 || probability > 1.0) {
        throw std::domain_error("Invalid risk-neutral probability; increase tree steps");
    }
    const double discount = std::exp(-option.rate * dt);

    std::vector<double> values(steps + 1);
    for (std::size_t j = 0; j <= steps; ++j) {
        const double exponent = static_cast<double>(steps) - 2.0 * static_cast<double>(j);
        values[j] = payoff(option.type, option.spot * std::pow(up, exponent), option.strike);
    }

    for (std::size_t level = steps; level-- > 0;) {
        for (std::size_t j = 0; j <= level; ++j) {
            values[j] = discount *
                        (probability * values[j] + (1.0 - probability) * values[j + 1]);
            if (style == ExerciseStyle::American) {
                const double exponent =
                    static_cast<double>(level) - 2.0 * static_cast<double>(j);
                const double spot = option.spot * std::pow(up, exponent);
                values[j] = std::max(values[j], payoff(option.type, spot, option.strike));
            }
        }
    }
    return values.front();
}

}  // namespace qf

