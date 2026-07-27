#pragma once

#include <span>
#include <vector>

namespace qf {

struct RiskMetrics {
    double value_at_risk{};
    double expected_shortfall{};
};

[[nodiscard]] RiskMetrics historical_risk(
    std::span<const double> profit_and_loss,
    double confidence_level = 0.95);

[[nodiscard]] std::vector<double> portfolio_pnl(
    std::span<const double> weights,
    std::span<const std::vector<double>> asset_returns,
    double portfolio_value);

}  // namespace qf

