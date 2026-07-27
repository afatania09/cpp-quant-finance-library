#pragma once

#include <cstddef>
#include <cstdint>

#include "quantfinance/types.hpp"

namespace qf {

struct MonteCarloResult {
    double price{};
    double standard_error{};
    double confidence_low{};
    double confidence_high{};
};

[[nodiscard]] MonteCarloResult monte_carlo_price(
    const Option& option,
    std::size_t paths = 100'000,
    std::uint64_t seed = 42);

}  // namespace qf

