#include <iomanip>
#include <iostream>

#include "quantfinance/binomial_tree.hpp"
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/fixed_income.hpp"
#include "quantfinance/heston.hpp"
#include "quantfinance/implied_volatility.hpp"
#include "quantfinance/monte_carlo.hpp"
#include "quantfinance/trinomial_tree.hpp"

int main() {
    const qf::Option call{
        .spot = 100.0,
        .strike = 100.0,
        .rate = 0.05,
        .dividend_yield = 0.02,
        .volatility = 0.20,
        .maturity = 1.0,
        .type = qf::OptionType::Call,
    };

    const double analytic = qf::black_scholes_price(call);
    const auto greeks = qf::black_scholes_greeks(call);
    const double lattice = qf::binomial_price(call, 1'000);
    const double trinomial = qf::trinomial_price(call, 1'000);
    const auto simulation = qf::monte_carlo_price(call, 200'000);
    const double recovered_volatility = qf::implied_volatility(analytic, call);
    const qf::HestonParameters heston{
        .mean_reversion = 2.0,
        .long_run_variance = 0.04,
        .volatility_of_variance = 0.30,
        .correlation = -0.70,
        .initial_variance = 0.04,
    };
    const auto heston_result = qf::heston_monte_carlo_price(call, heston, 50'000, 252);

    const qf::ZeroCurve curve(
        {0.5, 1.0, 2.0, 5.0, 10.0},
        {0.040, 0.041, 0.042, 0.044, 0.045});
    const double curve_bond = qf::coupon_bond_price(100.0, 0.05, 5.0, 2, curve);
    const auto bond = qf::coupon_bond_analytics(100.0, 0.05, 5.0, 2, 0.045);

    std::cout << std::fixed << std::setprecision(6)
              << "Black-Scholes price : " << analytic << '\n'
              << "Binomial price      : " << lattice << '\n'
              << "Trinomial price     : " << trinomial << '\n'
              << "Monte Carlo price   : " << simulation.price << " (95% CI "
              << simulation.confidence_low << ", " << simulation.confidence_high << ")\n"
              << "Heston MC price     : " << heston_result.price << " (SE "
              << heston_result.standard_error << ")\n"
              << "Delta / Gamma / Vega: " << greeks.delta << " / " << greeks.gamma
              << " / " << greeks.vega << '\n'
              << "Recovered volatility: " << recovered_volatility << '\n'
              << "Curve-priced bond   : " << curve_bond << '\n'
              << "Bond duration       : " << bond.macaulay_duration
              << " years; convexity: " << bond.convexity << '\n';
}
