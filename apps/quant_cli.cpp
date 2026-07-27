#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "quantfinance/binomial_tree.hpp"
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/fixed_income.hpp"
#include "quantfinance/heston.hpp"
#include "quantfinance/monte_carlo.hpp"
#include "quantfinance/trinomial_tree.hpp"

namespace {

template <typename T>
T read_value(const std::string& prompt) {
    T value{};
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            return value;
        }
        std::cout << "Invalid input. Please enter a number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

qf::Option read_option() {
    const double spot = read_value<double>("Spot price: ");
    const double strike = read_value<double>("Strike price: ");
    const double rate = read_value<double>("Risk-free rate (e.g. 0.05): ");
    const double dividend = read_value<double>("Dividend yield (e.g. 0.02): ");
    const double volatility = read_value<double>("Volatility (e.g. 0.20): ");
    const double maturity = read_value<double>("Maturity in years: ");
    const int type = read_value<int>("Option type (1 = call, 2 = put): ");
    return {
        .spot = spot,
        .strike = strike,
        .rate = rate,
        .dividend_yield = dividend,
        .volatility = volatility,
        .maturity = maturity,
        .type = type == 2 ? qf::OptionType::Put : qf::OptionType::Call,
    };
}

void price_vanilla_option() {
    const qf::Option option = read_option();
    const std::size_t steps = read_value<std::size_t>("Lattice steps (e.g. 1000): ");
    const std::size_t paths = read_value<std::size_t>("Monte Carlo paths (e.g. 500000): ");
    const auto greeks = qf::black_scholes_greeks(option);
    const auto simulation = qf::monte_carlo_price_parallel(option, paths);

    std::cout << "\n--- Vanilla option valuation ---\n"
              << "Black-Scholes : " << qf::black_scholes_price(option) << '\n'
              << "Binomial      : " << qf::binomial_price(option, steps) << '\n'
              << "Trinomial     : " << qf::trinomial_price(option, steps) << '\n'
              << "Parallel MC   : " << simulation.price << " (SE "
              << simulation.standard_error << ")\n"
              << "95% interval  : [" << simulation.confidence_low << ", "
              << simulation.confidence_high << "]\n"
              << "Delta         : " << greeks.delta << '\n'
              << "Gamma         : " << greeks.gamma << '\n'
              << "Vega          : " << greeks.vega << '\n'
              << "Theta         : " << greeks.theta << '\n'
              << "Rho           : " << greeks.rho << '\n';
}

void price_heston_option() {
    const qf::Option option = read_option();
    const qf::HestonParameters parameters{
        .mean_reversion = read_value<double>("Mean reversion kappa: "),
        .long_run_variance = read_value<double>("Long-run variance theta: "),
        .volatility_of_variance = read_value<double>("Volatility of variance sigma: "),
        .correlation = read_value<double>("Spot/variance correlation rho: "),
        .initial_variance = read_value<double>("Initial variance v0: "),
    };
    const std::size_t paths = read_value<std::size_t>("Simulation paths: ");
    const std::size_t steps = read_value<std::size_t>("Time steps per path: ");
    const auto result = qf::heston_monte_carlo_price(option, parameters, paths, steps);
    std::cout << "\n--- Heston valuation ---\n"
              << "Price         : " << result.price << '\n'
              << "Standard error: " << result.standard_error << '\n'
              << "95% interval  : [" << result.confidence_low << ", "
              << result.confidence_high << "]\n";
}

void price_bond() {
    const double face = read_value<double>("Face value: ");
    const double coupon = read_value<double>("Annual coupon rate (e.g. 0.05): ");
    const double maturity = read_value<double>("Maturity in years: ");
    const std::size_t frequency =
        read_value<std::size_t>("Payments per year (1, 2, 4 or 12): ");
    const double yield = read_value<double>("Annual yield to maturity: ");
    const auto result =
        qf::coupon_bond_analytics(face, coupon, maturity, frequency, yield);
    std::cout << "\n--- Bond valuation ---\n"
              << "Price             : " << result.price << '\n'
              << "Macaulay duration : " << result.macaulay_duration << '\n'
              << "Modified duration : " << result.modified_duration << '\n'
              << "Convexity         : " << result.convexity << '\n';
}

}  // namespace

int main() {
    std::cout << std::fixed << std::setprecision(6)
              << "C++ Quantitative Finance Calculator\n"
              << "1. Vanilla equity option\n"
              << "2. Heston stochastic-volatility option\n"
              << "3. Fixed-rate coupon bond\n";
    const int choice = read_value<int>("Select instrument: ");
    try {
        if (choice == 1) {
            price_vanilla_option();
        } else if (choice == 2) {
            price_heston_option();
        } else if (choice == 3) {
            price_bond();
        } else {
            std::cerr << "Unknown instrument selection.\n";
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "Pricing error: " << error.what() << '\n';
        return 1;
    }
}

