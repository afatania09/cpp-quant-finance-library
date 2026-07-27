#pragma once

#include <cstddef>
#include <cstdint>

#include "quantfinance/types.hpp"

namespace qf {

struct HestonParameters {
    double mean_reversion{};
    double long_run_variance{};
    double volatility_of_variance{};
    double correlation{};
    double initial_variance{};

    void validate() const;
};

struct HestonResult {
    double price{};
    double standard_error{};
    double confidence_low{};
    double confidence_high{};
};

[[nodiscard]] HestonResult heston_monte_carlo_price(
    const Option& option,
    const HestonParameters& parameters,
    std::size_t paths = 100'000,
    std::size_t time_steps = 252,
    std::uint64_t seed = 42);

}  // namespace qf

