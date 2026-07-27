#include "quantfinance/monte_carlo.hpp"

#include <cmath>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

#include "quantfinance/black_scholes.hpp"

namespace qf {
namespace {

struct SimulationMoments {
    double sum{};
    double sum_squares{};
    std::size_t observations{};
};

SimulationMoments simulate_chunk(
    const Option& option,
    std::size_t paths,
    std::uint64_t seed) {
    std::mt19937_64 engine(seed);
    std::normal_distribution<double> normal;
    const double drift = (option.rate - option.dividend_yield -
                          0.5 * option.volatility * option.volatility) *
                         option.maturity;
    const double diffusion = option.volatility * std::sqrt(option.maturity);
    const double discount = std::exp(-option.rate * option.maturity);
    SimulationMoments moments;

    while (moments.observations < paths) {
        const double z = normal(engine);
        for (const double shock : {z, -z}) {
            if (moments.observations == paths) {
                break;
            }
            const double terminal_spot = option.spot * std::exp(drift + diffusion * shock);
            const double discounted_payoff =
                discount * payoff(option.type, terminal_spot, option.strike);
            moments.sum += discounted_payoff;
            moments.sum_squares += discounted_payoff * discounted_payoff;
            ++moments.observations;
        }
    }
    return moments;
}

MonteCarloResult moments_to_result(const SimulationMoments& moments) {
    const double n = static_cast<double>(moments.observations);
    const double mean = moments.sum / n;
    const double sample_variance =
        (moments.sum_squares - n * mean * mean) / (n - 1.0);
    const double standard_error = std::sqrt(std::max(sample_variance, 0.0) / n);
    constexpr double z_975 = 1.959963984540054;
    return {
        mean,
        standard_error,
        mean - z_975 * standard_error,
        mean + z_975 * standard_error,
    };
}

}  // namespace

MonteCarloResult monte_carlo_price(
    const Option& option,
    std::size_t paths,
    std::uint64_t seed) {
    option.validate();
    if (paths < 2) {
        throw std::invalid_argument("Monte Carlo requires at least two paths");
    }
    if (option.maturity == 0.0) {
        const double value = payoff(option.type, option.spot, option.strike);
        return {value, 0.0, value, value};
    }

    return moments_to_result(simulate_chunk(option, paths, seed));
}

MonteCarloResult monte_carlo_price_parallel(
    const Option& option,
    std::size_t paths,
    std::size_t thread_count,
    std::uint64_t seed) {
    option.validate();
    if (paths < 2) {
        throw std::invalid_argument("Parallel Monte Carlo requires at least two paths");
    }
    if (option.maturity == 0.0) {
        const double value = payoff(option.type, option.spot, option.strike);
        return {value, 0.0, value, value};
    }
    if (thread_count == 0) {
        thread_count = std::max<std::size_t>(std::thread::hardware_concurrency(), 1);
    }
    thread_count = std::min(thread_count, paths / 2);

    std::vector<SimulationMoments> partial(thread_count);
    std::vector<std::thread> workers;
    workers.reserve(thread_count);
    const std::size_t base_paths = paths / thread_count;
    const std::size_t remainder = paths % thread_count;
    constexpr std::uint64_t seed_stride = 0x9E3779B97F4A7C15ULL;

    for (std::size_t worker = 0; worker < thread_count; ++worker) {
        const std::size_t worker_paths = base_paths + (worker < remainder ? 1 : 0);
        workers.emplace_back([&, worker, worker_paths] {
            partial[worker] =
                simulate_chunk(option, worker_paths, seed + seed_stride * worker);
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    SimulationMoments total;
    for (const auto& moments : partial) {
        total.sum += moments.sum;
        total.sum_squares += moments.sum_squares;
        total.observations += moments.observations;
    }
    return moments_to_result(total);
}

}  // namespace qf
