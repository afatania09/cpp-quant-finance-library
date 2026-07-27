#pragma once

#include <cstddef>

#include "quantfinance/types.hpp"

namespace qf {

[[nodiscard]] double implied_volatility(
    double market_price,
    Option option,
    double tolerance = 1e-8,
    std::size_t max_iterations = 100);

}  // namespace qf

