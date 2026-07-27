#include "quantfinance/trinomial_tree.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "quantfinance/black_scholes.hpp"

namespace qf {

double trinomial_price(const Option& option, std::size_t steps, ExerciseStyle style) {
    option.validate();
    if (steps == 0) {
        throw std::invalid_argument("Trinomial tree requires at least one step");
    }
    if (option.maturity == 0.0) {
        return payoff(option.type, option.spot, option.strike);
    }
    if (option.volatility == 0.0) {
        return black_scholes_price(option);
    }

    const double dt = option.maturity / static_cast<double>(steps);
    const double dx = option.volatility * std::sqrt(3.0 * dt);
    const double variance = option.volatility * option.volatility;
    const double drift = option.rate - option.dividend_yield - 0.5 * variance;
    const double common = (variance * dt + drift * drift * dt * dt) / (dx * dx);
    const double directional = drift * dt / dx;
    const double probability_up = 0.5 * (common + directional);
    const double probability_down = 0.5 * (common - directional);
    const double probability_middle = 1.0 - probability_up - probability_down;
    if (probability_up < 0.0 || probability_down < 0.0 || probability_middle < 0.0) {
        throw std::domain_error("Invalid trinomial probabilities; increase tree steps");
    }

    std::vector<double> values(2 * steps + 1);
    const auto n = static_cast<long long>(steps);
    for (long long state = -n; state <= n; ++state) {
        const double terminal_spot =
            option.spot * std::exp(static_cast<double>(state) * dx);
        values[static_cast<std::size_t>(state + n)] =
            payoff(option.type, terminal_spot, option.strike);
    }

    const double discount = std::exp(-option.rate * dt);
    for (std::size_t level = steps; level-- > 0;) {
        const auto width = static_cast<long long>(level);
        for (long long state = -width; state <= width; ++state) {
            const auto down_index = static_cast<std::size_t>(state + width);
            const auto middle_index = down_index + 1;
            const auto up_index = down_index + 2;
            double value = discount *
                           (probability_down * values[down_index] +
                            probability_middle * values[middle_index] +
                            probability_up * values[up_index]);
            if (style == ExerciseStyle::American) {
                const double node_spot =
                    option.spot * std::exp(static_cast<double>(state) * dx);
                value = std::max(value, payoff(option.type, node_spot, option.strike));
            }
            values[static_cast<std::size_t>(state + width)] = value;
        }
    }
    return values.front();
}

}  // namespace qf

