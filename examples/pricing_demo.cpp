#include <iomanip>
#include <iostream>

#include "quantfinance/binomial_tree.hpp"
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/implied_volatility.hpp"
#include "quantfinance/monte_carlo.hpp"

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
    const auto simulation = qf::monte_carlo_price(call, 200'000);
    const double recovered_volatility = qf::implied_volatility(analytic, call);

    std::cout << std::fixed << std::setprecision(6)
              << "Black-Scholes price : " << analytic << '\n'
              << "Binomial price      : " << lattice << '\n'
              << "Monte Carlo price   : " << simulation.price << " (95% CI "
              << simulation.confidence_low << ", " << simulation.confidence_high << ")\n"
              << "Delta / Gamma / Vega: " << greeks.delta << " / " << greeks.gamma
              << " / " << greeks.vega << '\n'
              << "Recovered volatility: " << recovered_volatility << '\n';
}

