#include "quantfinance/black_scholes.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace qf {
namespace {

struct DTerms {
    double d1;
    double d2;
};

DTerms d_terms(const Option& o) {
    const double root_t = std::sqrt(o.maturity);
    const double d1 = (std::log(o.spot / o.strike) +
                       (o.rate - o.dividend_yield + 0.5 * o.volatility * o.volatility) *
                           o.maturity) /
                      (o.volatility * root_t);
    return {d1, d1 - o.volatility * root_t};
}

}  // namespace

double normal_cdf(double x) noexcept {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double normal_pdf(double x) noexcept {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * std::numbers::pi);
}

double payoff(OptionType type, double spot, double strike) noexcept {
    return type == OptionType::Call ? std::max(spot - strike, 0.0)
                                    : std::max(strike - spot, 0.0);
}

double black_scholes_price(const Option& option) {
    option.validate();
    if (option.maturity == 0.0) {
        return payoff(option.type, option.spot, option.strike);
    }
    if (option.volatility == 0.0) {
        const double forward_spot =
            option.spot * std::exp((option.rate - option.dividend_yield) * option.maturity);
        return std::exp(-option.rate * option.maturity) *
               payoff(option.type, forward_spot, option.strike);
    }

    const auto [d1, d2] = d_terms(option);
    const double discounted_spot =
        option.spot * std::exp(-option.dividend_yield * option.maturity);
    const double discounted_strike =
        option.strike * std::exp(-option.rate * option.maturity);

    if (option.type == OptionType::Call) {
        return discounted_spot * normal_cdf(d1) - discounted_strike * normal_cdf(d2);
    }
    return discounted_strike * normal_cdf(-d2) - discounted_spot * normal_cdf(-d1);
}

Greeks black_scholes_greeks(const Option& option) {
    option.validate();
    if (option.maturity <= 0.0 || option.volatility <= 0.0) {
        throw std::invalid_argument("Greeks require positive maturity and volatility");
    }

    const auto [d1, d2] = d_terms(option);
    const double root_t = std::sqrt(option.maturity);
    const double discount_q = std::exp(-option.dividend_yield * option.maturity);
    const double discount_r = std::exp(-option.rate * option.maturity);
    const double sign = option.type == OptionType::Call ? 1.0 : -1.0;

    Greeks result;
    result.delta = sign * discount_q * normal_cdf(sign * d1);
    result.gamma = discount_q * normal_pdf(d1) /
                   (option.spot * option.volatility * root_t);
    result.vega = option.spot * discount_q * normal_pdf(d1) * root_t;

    const double diffusion =
        -(option.spot * discount_q * normal_pdf(d1) * option.volatility) / (2.0 * root_t);
    if (option.type == OptionType::Call) {
        result.theta = diffusion -
                       option.rate * option.strike * discount_r * normal_cdf(d2) +
                       option.dividend_yield * option.spot * discount_q * normal_cdf(d1);
        result.rho = option.strike * option.maturity * discount_r * normal_cdf(d2);
    } else {
        result.theta = diffusion +
                       option.rate * option.strike * discount_r * normal_cdf(-d2) -
                       option.dividend_yield * option.spot * discount_q * normal_cdf(-d1);
        result.rho = -option.strike * option.maturity * discount_r * normal_cdf(-d2);
    }
    return result;
}

}  // namespace qf

