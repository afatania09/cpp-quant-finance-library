#pragma once

#include <cstddef>

#include "quantfinance/types.hpp"

namespace qf {

[[nodiscard]] double trinomial_price(
    const Option& option,
    std::size_t steps,
    ExerciseStyle style = ExerciseStyle::European);

}  // namespace qf

