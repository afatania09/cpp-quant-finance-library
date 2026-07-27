#pragma once

#include "quantfinance/types.hpp"

namespace qf {

[[nodiscard]] double normal_cdf(double x) noexcept;
[[nodiscard]] double normal_pdf(double x) noexcept;
[[nodiscard]] double payoff(OptionType type, double spot, double strike) noexcept;
[[nodiscard]] double black_scholes_price(const Option& option);
[[nodiscard]] Greeks black_scholes_greeks(const Option& option);

}  // namespace qf

