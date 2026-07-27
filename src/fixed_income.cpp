#include "quantfinance/fixed_income.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace qf {
namespace {

std::size_t payment_count(double maturity, std::size_t frequency) {
    if (maturity <= 0.0 || frequency == 0) {
        throw std::invalid_argument("Maturity and payment frequency must be positive");
    }
    const double raw_count = maturity * static_cast<double>(frequency);
    const auto count = static_cast<std::size_t>(std::llround(raw_count));
    if (std::abs(raw_count - static_cast<double>(count)) > 1e-10) {
        throw std::invalid_argument("Maturity must align with the coupon frequency");
    }
    return count;
}

void validate_bond(double face_value, double coupon_rate) {
    if (face_value <= 0.0 || coupon_rate < 0.0) {
        throw std::invalid_argument("Invalid bond cash-flow parameters");
    }
}

}  // namespace

ZeroCurve::ZeroCurve(
    std::vector<double> maturities,
    std::vector<double> continuous_rates)
    : maturities_(std::move(maturities)), rates_(std::move(continuous_rates)) {
    if (maturities_.empty() || maturities_.size() != rates_.size()) {
        throw std::invalid_argument("Curve pillars and rates must have equal non-zero size");
    }
    if (!std::is_sorted(maturities_.begin(), maturities_.end()) ||
        maturities_.front() <= 0.0 ||
        std::adjacent_find(maturities_.begin(), maturities_.end()) != maturities_.end()) {
        throw std::invalid_argument("Curve maturities must be strictly increasing and positive");
    }
}

double ZeroCurve::zero_rate(double maturity) const {
    if (maturity < 0.0) {
        throw std::invalid_argument("Maturity cannot be negative");
    }
    if (maturity <= maturities_.front()) {
        return rates_.front();
    }
    if (maturity >= maturities_.back()) {
        return rates_.back();
    }
    const auto upper = std::upper_bound(maturities_.begin(), maturities_.end(), maturity);
    const auto right = static_cast<std::size_t>(upper - maturities_.begin());
    const auto left = right - 1;
    const double weight =
        (maturity - maturities_[left]) / (maturities_[right] - maturities_[left]);
    return rates_[left] + weight * (rates_[right] - rates_[left]);
}

double ZeroCurve::discount_factor(double maturity) const {
    return std::exp(-zero_rate(maturity) * maturity);
}

BondAnalytics coupon_bond_analytics(
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    double annual_yield) {
    validate_bond(face_value, annual_coupon_rate);
    const std::size_t payments = payment_count(maturity, payments_per_year);
    const double frequency = static_cast<double>(payments_per_year);
    const double periodic_yield = annual_yield / frequency;
    if (periodic_yield <= -1.0) {
        throw std::invalid_argument("Periodic yield must be greater than -100%");
    }
    const double coupon = face_value * annual_coupon_rate / frequency;

    double price = 0.0;
    double duration_numerator = 0.0;
    double convexity_numerator = 0.0;
    for (std::size_t payment = 1; payment <= payments; ++payment) {
        const double time = static_cast<double>(payment) / frequency;
        const double cash_flow = coupon + (payment == payments ? face_value : 0.0);
        const double discount =
            std::pow(1.0 + periodic_yield, -static_cast<double>(payment));
        const double present_value = cash_flow * discount;
        price += present_value;
        duration_numerator += time * present_value;
        convexity_numerator +=
            static_cast<double>(payment * (payment + 1)) * present_value;
    }
    const double macaulay = duration_numerator / price;
    const double modified = macaulay / (1.0 + periodic_yield);
    const double convexity =
        convexity_numerator /
        (price * frequency * frequency *
         (1.0 + periodic_yield) * (1.0 + periodic_yield));
    return {price, macaulay, modified, convexity};
}

double coupon_bond_price(
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    const ZeroCurve& curve) {
    validate_bond(face_value, annual_coupon_rate);
    const std::size_t payments = payment_count(maturity, payments_per_year);
    const double frequency = static_cast<double>(payments_per_year);
    const double coupon = face_value * annual_coupon_rate / frequency;

    double price = 0.0;
    for (std::size_t payment = 1; payment <= payments; ++payment) {
        const double time = static_cast<double>(payment) / frequency;
        const double cash_flow = coupon + (payment == payments ? face_value : 0.0);
        price += cash_flow * curve.discount_factor(time);
    }
    return price;
}

double yield_to_maturity(
    double market_price,
    double face_value,
    double annual_coupon_rate,
    double maturity,
    std::size_t payments_per_year,
    double tolerance) {
    if (market_price <= 0.0 || tolerance <= 0.0) {
        throw std::invalid_argument("Market price and tolerance must be positive");
    }
    double low = -0.99 * static_cast<double>(payments_per_year);
    double high = 10.0;
    for (std::size_t iteration = 0; iteration < 250; ++iteration) {
        const double candidate = 0.5 * (low + high);
        const double price = coupon_bond_analytics(
                                 face_value,
                                 annual_coupon_rate,
                                 maturity,
                                 payments_per_year,
                                 candidate)
                                 .price;
        if (std::abs(price - market_price) < tolerance) {
            return candidate;
        }
        if (price > market_price) {
            low = candidate;
        } else {
            high = candidate;
        }
    }
    throw std::runtime_error("Yield-to-maturity solver did not converge");
}

}  // namespace qf

