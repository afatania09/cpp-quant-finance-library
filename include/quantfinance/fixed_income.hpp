#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace qf {

class ZeroCurve {
public:
    ZeroCurve(std::vector<double> maturities, std::vector<double> continuous_rates);

    [[nodiscard]] double zero_rate(double maturity) const;
    [[nodiscard]] double discount_factor(double maturity) const;

private:
    std::vector<double> maturities_;
    std::vector<double> rates_;
};

struct BondAnalytics {
    double price{};
    double macaulay_duration{};
    double modified_duration{};
    double convexity{};
};

[[nodiscard]] BondAnalytics coupon_bond_analytics(
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    double annual_yield);

[[nodiscard]] double coupon_bond_price(
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    const ZeroCurve& curve);

[[nodiscard]] double yield_to_maturity(
    double market_price,
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    double tolerance = 1e-10);

}  // namespace qf

