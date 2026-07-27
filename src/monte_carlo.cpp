#include "quantfinance/monte_carlo.hpp"

#include <cmath>
#include <random>
#include <stdexcept>

#include "quantfinance/black_scholes.hpp"

namespace qf {

MonteCarloResult monte_carlo_price(
    const Option& option,
    std::size_t paths,
    std::uint64_t seed) {
    option.validate();
    if (paths < 2) {
        throw std::invalid_argument("Monte Carlo requires at least two paths");
    }
    if (option.maturity == 0.0) {
        const double value = payoff(option.type, option.spot, option.strike);
        return {value, 0.0, value, value};
    }

    std::mt19937_64 engine(seed);
    std::normal_distribution<double> normal;
    const double drift = (option.rate - option.dividend_yield -
                          0.5 * option.volatility * option.volatility) *
                         option.maturity;
    const double diffusion = option.volatility * std::sqrt(option.maturity);
    const double discount = std::exp(-option.rate * option.maturity);

    double sum = 0.0;
    double sum_squares = 0.0;
    std::size_t observations = 0;

    while (observations < paths) {
        const double z = normal(engine);
        for (const double shock : {z, -z}) {
            if (observations == paths) {
                break;
            }
            const double terminal_spot = option.spot * std::exp(drift + diffusion * shock);
            const double discounted_payoff =
                discount * payoff(option.type, terminal_spot, option.strike);
            sum += discounted_payoff;
            sum_squares += discounted_payoff * discounted_payoff;
            ++observations;
        }
    }

    const double n = static_cast<double>(observations);
    const double mean = sum / n;
    const double sample_variance = (sum_squares - n * mean * mean) / (n - 1.0);
    const double standard_error = std::sqrt(std::max(sample_variance, 0.0) / n);
    constexpr double z_975 = 1.959963984540054;
    return {
        mean,
        standard_error,
        mean - z_975 * standard_error,
        mean + z_975 * standard_error,
    };
}

}  // namespace qf

