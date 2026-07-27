# C++ Quantitative Finance Library

[![Manual C++ CI](https://github.com/afatania09/cpp-quant-finance-library/actions/workflows/ci.yml/badge.svg)](https://github.com/afatania09/cpp-quant-finance-library/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modern C++20 library demonstrating derivatives pricing, numerical methods and
market-risk analytics. The project is designed as an auditable quantitative
finance portfolio: every model has a clear interface, deterministic examples,
numerical validation and cross-platform continuous integration.

Version 3 adds a usable terminal calculator, parallel simulation, explicit
convergence experiments and documented model limitations.

## Implemented models

| Module | Methods and outputs |
|---|---|
| Black–Scholes–Merton | European calls and puts with continuous dividends |
| Analytic Greeks | Delta, gamma, vega, theta and rho |
| Cox–Ross–Rubinstein tree | European and American exercise |
| Recombining trinomial tree | European and American exercise with three-state transitions |
| Monte Carlo | Serial and multithreaded GBM, antithetic variates, standard error and 95% confidence interval |
| Heston simulation | Correlated spot/variance paths with full-truncation Euler discretisation |
| Implied volatility | Robust bisection subject to no-arbitrage price bounds |
| Fixed income | Zero curves, coupon bonds, YTM, duration and convexity |
| Portfolio risk | Historical P&L, value at risk and expected shortfall |

## Architecture

```text
include/quantfinance/   Public, reusable API
src/                    Model implementations
examples/               Executable pricing demonstration
apps/                   Interactive terminal pricing calculator
benchmarks/             Runtime and numerical-convergence experiments
docs/                   Validation evidence, assumptions and limitations
tests/                  Numerical and financial invariants
.github/workflows/      Linux, macOS and Windows CI
```

The library separates financial contracts from pricing algorithms. It uses
standard-library facilities only, so it has no third-party runtime dependency.

## Build and run

Requirements: a C++20 compiler and CMake 3.20 or newer.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run the example on Linux or macOS:

```bash
./build/pricing_demo
```

Price your own option or bond interactively:

```bash
./build/quant_cli
```

Generate convergence and runtime evidence:

```bash
./build/pricing_benchmark
```

On a multi-configuration Windows build:

```powershell
.\build\Release\pricing_demo.exe
```

## Example API

```cpp
#include "quantfinance/black_scholes.hpp"
#include "quantfinance/monte_carlo.hpp"

qf::Option call{
    .spot = 100.0,
    .strike = 100.0,
    .rate = 0.05,
    .dividend_yield = 0.02,
    .volatility = 0.20,
    .maturity = 1.0,
    .type = qf::OptionType::Call,
};

const double price = qf::black_scholes_price(call);
const qf::Greeks greeks = qf::black_scholes_greeks(call);
const qf::MonteCarloResult simulation = qf::monte_carlo_price(call);
```

For the parameters above, the analytic call value is approximately `9.227006`.
The test suite independently checks this reference value, put–call parity,
lattice convergence, American-option dominance, implied-volatility recovery,
Greek signs, Monte Carlo confidence intervals, Heston reproducibility, par-bond
pricing, yield recovery, curve interpolation and portfolio-risk invariants.

## Numerical choices

- The lattice uses the Cox–Ross–Rubinstein parametrisation and backward induction.
- The simulation uses a fixed seed by default for reproducible tests and
  antithetic normal shocks for variance reduction.
- The parallel simulation divides paths across standard C++ worker threads,
  maintains per-thread moments and combines them after joining.
- Implied volatility uses bisection rather than unconstrained Newton iterations,
  prioritising reliable convergence for a portfolio demonstration.
- Historical risk is reported as positive loss magnitudes.
- The zero curve linearly interpolates continuously compounded zero rates and
  discounts each bond cash flow at its corresponding maturity.
- Heston variance is advanced with a full-truncation Euler scheme to prevent
  negative variance entering diffusion terms.

## Roadmap

- Finite-difference PDE engine
- Heston semi-analytic pricing and calibration
- Bootstrapped curves, floating-rate instruments and interest-rate swaps
- Sensitivity benchmarking and parallel Monte Carlo
- Python bindings for comparison with the native C++ engine

## Validation

The project distinguishes sampling error, discretisation error and model risk.
See [model validation and limitations](docs/MODEL_VALIDATION.md) for the
automated validation matrix, assumptions, known limitations and conditions
under which the library should not be used.

Continuous integration is started deliberately from GitHub's **Actions** tab
rather than on every commit. This avoids repeated builds and email notifications
while related connector-backed file updates are published as successive commits.

## Disclaimer

This project is for education, research and portfolio demonstration. It is not
investment advice and is not validated for production trading or risk reporting.
