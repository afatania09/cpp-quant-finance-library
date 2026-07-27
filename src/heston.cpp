#include "quantfinance/heston.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

#include "quantfinance/black_scholes.hpp"

namespace qf {

void HestonParameters::validate() const {
    if (mean_reversion < 0.0 || long_run_variance < 0.0 ||
        volatility_of_variance < 0.0 || initial_variance < 0.0) {
        throw std::invalid_argument("Heston variance parameters cannot be negative");
    }
    if (correlation < -1.0 || correlation > 1.0) {
        throw std::invalid_argument("Heston correlation must lie in [-1, 1]");
    }
}

HestonResult heston_monte_carlo_price(
    const Option& option,
    const HestonParameters& parameters,
    std::size_t paths,
    std::size_t time_steps,
    std::uint64_t seed) {
    option.validate();
    parameters.validate();
    if (paths < 2 || time_steps == 0) {
        throw std::invalid_argument("Heston simulation requires two paths and one time step");
    }
    if (option.maturity == 0.0) {
        const double value = payoff(option.type, option.spot, option.strike);
        return {value, 0.0, value, value};
    }

    std::mt19937_64 engine(seed);
    std::normal_distribution<double> normal;
    const double dt = option.maturity / static_cast<double>(time_steps);
    const double root_dt = std::sqrt(dt);
    const double independent_scale =
        std::sqrt(std::max(1.0 - parameters.correlation * parameters.correlation, 0.0));
    const double discount = std::exp(-option.rate * option.maturity);

    double sum = 0.0;
    double sum_squares = 0.0;
    for (std::size_t path = 0; path < paths; ++path) {
        double log_spot = std::log(option.spot);
        double variance = parameters.initial_variance;
        for (std::size_t step = 0; step < time_steps; ++step) {
            const double z_spot = normal(engine);
            const double z_independent = normal(engine);
            const double z_variance =
                parameters.correlation * z_spot + independent_scale * z_independent;
            const double variance_positive = std::max(variance, 0.0);

            log_spot +=
                (option.rate - option.dividend_yield - 0.5 * variance_positive) * dt +
                std::sqrt(variance_positive) * root_dt * z_spot;
            variance +=
                parameters.mean_reversion *
                    (parameters.long_run_variance - variance_positive) * dt +
                parameters.volatility_of_variance * std::sqrt(variance_positive) *
                    root_dt * z_variance;
            variance = std::max(variance, 0.0);
        }
        const double discounted_payoff =
            discount * payoff(option.type, std::exp(log_spot), option.strike);
        sum += discounted_payoff;
        sum_squares += discounted_payoff * discounted_payoff;
    }

    const double n = static_cast<double>(paths);
    const double mean = sum / n;
    const double variance =
        std::max((sum_squares - n * mean * mean) / (n - 1.0), 0.0);
    const double standard_error = std::sqrt(variance / n);
    constexpr double z_975 = 1.959963984540054;
    return {
        mean,
        standard_error,
        mean - z_975 * standard_error,
        mean + z_975 * standard_error,
    };
}

}  // namespace qf

