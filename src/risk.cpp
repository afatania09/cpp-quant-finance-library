#include "quantfinance/risk.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace qf {

RiskMetrics historical_risk(
    std::span<const double> profit_and_loss,
    double confidence_level) {
    if (profit_and_loss.empty() || confidence_level <= 0.0 || confidence_level >= 1.0) {
        throw std::invalid_argument("Invalid historical-risk input");
    }

    std::vector<double> losses;
    losses.reserve(profit_and_loss.size());
    for (const double pnl : profit_and_loss) {
        losses.push_back(-pnl);
    }
    std::sort(losses.begin(), losses.end());

    const double raw_index =
        confidence_level * static_cast<double>(losses.size() - 1);
    const auto index = static_cast<std::size_t>(std::ceil(raw_index));
    const double value_at_risk = std::max(losses[index], 0.0);

    double tail_sum = 0.0;
    std::size_t tail_count = 0;
    for (std::size_t i = index; i < losses.size(); ++i) {
        tail_sum += losses[i];
        ++tail_count;
    }
    return {value_at_risk, std::max(tail_sum / static_cast<double>(tail_count), 0.0)};
}

std::vector<double> portfolio_pnl(
    std::span<const double> weights,
    std::span<const std::vector<double>> asset_returns,
    double portfolio_value) {
    if (weights.empty() || weights.size() != asset_returns.size() || portfolio_value <= 0.0) {
        throw std::invalid_argument("Invalid portfolio input");
    }
    const std::size_t observations = asset_returns.front().size();
    if (observations == 0) {
        throw std::invalid_argument("Asset return histories cannot be empty");
    }
    for (const auto& returns : asset_returns) {
        if (returns.size() != observations) {
            throw std::invalid_argument("Asset return histories must have equal length");
        }
    }

    std::vector<double> pnl(observations, 0.0);
    for (std::size_t asset = 0; asset < weights.size(); ++asset) {
        for (std::size_t observation = 0; observation < observations; ++observation) {
            pnl[observation] +=
                portfolio_value * weights[asset] * asset_returns[asset][observation];
        }
    }
    return pnl;
}

}  // namespace qf

