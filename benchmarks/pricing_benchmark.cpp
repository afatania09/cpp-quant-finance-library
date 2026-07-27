#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string_view>

#include "quantfinance/binomial_tree.hpp"
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/heston.hpp"
#include "quantfinance/monte_carlo.hpp"
#include "quantfinance/trinomial_tree.hpp"

namespace {

template <typename Function>
double timed_milliseconds(std::string_view label, Function&& function) {
    const auto start = std::chrono::steady_clock::now();
    const double result = function();
    const auto stop = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration<double, std::milli>(stop - start).count();
    std::cout << std::left << std::setw(28) << label << std::right << std::setw(12)
              << elapsed << " ms  result " << result << '\n';
    return result;
}

}  // namespace

int main() {
    const qf::Option option{
        .spot = 100.0,
        .strike = 100.0,
        .rate = 0.05,
        .dividend_yield = 0.02,
        .volatility = 0.20,
        .maturity = 1.0,
        .type = qf::OptionType::Call,
    };
    const double reference = qf::black_scholes_price(option);

    std::cout << std::fixed << std::setprecision(6)
              << "Convergence against Black-Scholes reference " << reference << "\n\n"
              << "Steps       Binomial       Abs error      Trinomial      Abs error\n";
    for (const std::size_t steps : std::array<std::size_t, 6>{25, 50, 100, 250, 500, 1'000}) {
        const double binomial = qf::binomial_price(option, steps);
        const double trinomial = qf::trinomial_price(option, steps);
        std::cout << std::setw(5) << steps << std::setw(15) << binomial
                  << std::setw(15) << std::abs(binomial - reference)
                  << std::setw(15) << trinomial << std::setw(15)
                  << std::abs(trinomial - reference) << '\n';
    }

    std::cout << "\nRuntime benchmark (hardware-dependent)\n";
    volatile double analytic_sink = 0.0;
    timed_milliseconds("1,000,000 Black-Scholes", [&] {
        double sum = 0.0;
        for (std::size_t iteration = 0; iteration < 1'000'000; ++iteration) {
            sum += qf::black_scholes_price(option);
        }
        analytic_sink = sum;
        return sum;
    });
    timed_milliseconds("1,000-step binomial", [&] {
        return qf::binomial_price(option, 1'000);
    });
    timed_milliseconds("1,000-step trinomial", [&] {
        return qf::trinomial_price(option, 1'000);
    });
    timed_milliseconds("1,000,000-path serial MC", [&] {
        return qf::monte_carlo_price(option, 1'000'000, 7).price;
    });
    timed_milliseconds("1,000,000-path parallel MC", [&] {
        return qf::monte_carlo_price_parallel(option, 1'000'000, 0, 7).price;
    });

    const qf::HestonParameters heston{
        .mean_reversion = 2.0,
        .long_run_variance = 0.04,
        .volatility_of_variance = 0.30,
        .correlation = -0.70,
        .initial_variance = 0.04,
    };
    timed_milliseconds("50,000-path Heston MC", [&] {
        return qf::heston_monte_carlo_price(option, heston, 50'000, 252, 7).price;
    });
    return analytic_sink > 0.0 ? 0 : 1;
}

