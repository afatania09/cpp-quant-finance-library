#include <cmath>
#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

#include "quantfinance/binomial_tree.hpp"
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/fixed_income.hpp"
#include "quantfinance/heston.hpp"
#include "quantfinance/implied_volatility.hpp"
#include "quantfinance/monte_carlo.hpp"
#include "quantfinance/risk.hpp"
#include "quantfinance/trinomial_tree.hpp"

namespace {

int failures = 0;

void expect_near(double actual, double expected, double tolerance, std::string_view name) {
    if (std::abs(actual - expected) > tolerance) {
        std::cerr << "FAIL: " << name << " expected " << expected << ", got " << actual << '\n';
        ++failures;
    }
}

void expect_true(bool condition, std::string_view name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}

qf::Option standard_option(qf::OptionType type) {
    return {
        .spot = 100.0,
        .strike = 100.0,
        .rate = 0.05,
        .dividend_yield = 0.02,
        .volatility = 0.20,
        .maturity = 1.0,
        .type = type,
    };
}

}  // namespace

int main() {
    try {
        const auto call = standard_option(qf::OptionType::Call);
        const auto put = standard_option(qf::OptionType::Put);
        const double call_price = qf::black_scholes_price(call);
        const double put_price = qf::black_scholes_price(put);

        expect_near(call_price, 9.227006, 1e-6, "Black-Scholes call reference");
        expect_near(put_price, 6.330081, 1e-6, "Black-Scholes put reference");

        const double parity_left = call_price - put_price;
        const double parity_right =
            call.spot * std::exp(-call.dividend_yield * call.maturity) -
            call.strike * std::exp(-call.rate * call.maturity);
        expect_near(parity_left, parity_right, 1e-10, "put-call parity");

        expect_near(qf::binomial_price(call, 2'000), call_price, 2e-3, "binomial convergence");
        expect_near(
            qf::trinomial_price(call, 1'000),
            call_price,
            2e-3,
            "trinomial convergence");
        expect_true(
            qf::binomial_price(put, 1'000, qf::ExerciseStyle::American) >= put_price,
            "American put dominates European put");
        expect_true(
            qf::trinomial_price(put, 500, qf::ExerciseStyle::American) >= put_price,
            "trinomial American put dominates European put");

        const auto mc = qf::monte_carlo_price(call, 400'000, 7);
        expect_true(
            call_price >= mc.confidence_low && call_price <= mc.confidence_high,
            "analytic price lies in Monte Carlo confidence interval");

        expect_near(
            qf::implied_volatility(call_price, call),
            call.volatility,
            1e-7,
            "implied volatility recovery");

        const auto greeks = qf::black_scholes_greeks(call);
        expect_true(greeks.delta > 0.0 && greeks.delta < 1.0, "call delta bounds");
        expect_true(greeks.gamma > 0.0 && greeks.vega > 0.0, "gamma and vega signs");

        const qf::HestonParameters heston{
            .mean_reversion = 2.0,
            .long_run_variance = 0.04,
            .volatility_of_variance = 0.30,
            .correlation = -0.70,
            .initial_variance = 0.04,
        };
        const auto heston_a = qf::heston_monte_carlo_price(call, heston, 20'000, 100, 19);
        const auto heston_b = qf::heston_monte_carlo_price(call, heston, 20'000, 100, 19);
        expect_near(heston_a.price, heston_b.price, 0.0, "Heston reproducibility");
        expect_true(
            heston_a.price > 0.0 && heston_a.standard_error > 0.0,
            "Heston price and error are positive");

        const auto par_bond = qf::coupon_bond_analytics(100.0, 0.05, 5.0, 2, 0.05);
        expect_near(par_bond.price, 100.0, 1e-10, "par bond pricing");
        expect_near(
            qf::yield_to_maturity(par_bond.price, 100.0, 0.05, 5.0, 2),
            0.05,
            1e-9,
            "yield-to-maturity recovery");
        expect_true(
            par_bond.macaulay_duration > 0.0 &&
                par_bond.modified_duration < par_bond.macaulay_duration &&
                par_bond.convexity > 0.0,
            "bond risk measures");

        const qf::ZeroCurve curve({1.0, 2.0, 5.0}, {0.03, 0.04, 0.05});
        expect_near(curve.zero_rate(1.5), 0.035, 1e-12, "zero-curve interpolation");
        expect_true(
            qf::coupon_bond_price(100.0, 0.04, 5.0, 2, curve) > 0.0,
            "curve-based bond pricing");

        const std::vector<double> weights{0.6, 0.4};
        const std::vector<std::vector<double>> returns{
            {0.01, -0.02, 0.005, -0.04, 0.015},
            {0.005, -0.01, 0.004, -0.02, 0.01},
        };
        const auto pnl = qf::portfolio_pnl(weights, returns, 1'000'000.0);
        expect_near(pnl[0], 8'000.0, 1e-10, "portfolio P&L aggregation");
        const auto risk = qf::historical_risk(pnl, 0.80);
        expect_true(
            risk.expected_shortfall >= risk.value_at_risk,
            "expected shortfall dominates VaR");
    } catch (const std::exception& error) {
        std::cerr << "Unexpected exception: " << error.what() << '\n';
        return 1;
    }

    if (failures == 0) {
        std::cout << "All quantitative-finance tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
