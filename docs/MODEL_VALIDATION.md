# Model validation and limitations

This document records what the library verifies, what it assumes and what it
does **not** claim. Reproducible limitations are part of a credible numerical
finance project.

## Validation matrix

| Model | Reference or invariant | Automated check |
|---|---|---|
| Black–Scholes | Published at-the-money reference value | Call and put within `1e-6` |
| European calls and puts | Put–call parity | Difference below `1e-10` |
| Binomial tree | Convergence to Black–Scholes | 2,000-step error below `0.002` |
| Trinomial tree | Convergence to Black–Scholes | 1,000-step error below `0.002` |
| American put | Early exercise cannot reduce value | American price ≥ European price |
| GBM Monte Carlo | Analytic value within sampled uncertainty | Black–Scholes lies inside 95% CI |
| Implied volatility | Recover known input volatility | Error below `1e-7` |
| Heston Monte Carlo | Seeded reproducibility and finite output | Exact repeat and positive price/SE |
| Coupon bond | Coupon equals yield | Price equals par |
| YTM solver | Recover yield from generated market price | Error below `1e-9` |
| Zero curve | Linear interpolation | Midpoint rate reference |
| Historical risk | Tail-loss ordering | Expected shortfall ≥ VaR |

Run `pricing_benchmark` to print lattice convergence at increasing step counts
and hardware-specific runtimes for analytic, lattice and simulation methods.

## Modelling assumptions

### Black–Scholes–Merton

The model assumes lognormal prices, constant volatility and rates, continuous
trading and frictionless markets. These assumptions are useful for benchmarking
but do not reproduce volatility smiles, jumps, liquidity constraints or
discrete hedging error.

### Lattice engines

The binomial and trinomial implementations use recombining trees with constant
parameters. Large step counts reduce discretisation error but increase runtime.
The convergence report should be considered alongside, rather than replaced by,
a single final price.

### GBM Monte Carlo

The engine uses pseudo-random normal variates and antithetic sampling.
Confidence intervals quantify sampling error, not model risk or discretisation
risk. The parallel engine assigns independent deterministic seeds to workers;
changing the thread count changes the random sample and therefore the estimate.

### Heston Monte Carlo

Variance uses full-truncation Euler discretisation. This prevents negative
variance entering square roots, but finite time steps still create
discretisation bias. The current engine has no calibration routine and should
not be represented as a production Heston implementation until it is compared
with a semi-analytic reference over a parameter grid.

### Fixed income

Coupon periods are regular and must align exactly with maturity. The simple zero
curve interpolates continuously compounded rates; it is supplied directly and
is not bootstrapped from deposits, futures and swaps. Day-count conventions,
calendars, settlement, accrued interest, credit risk and optionality are outside
the current scope.

### Historical risk

Historical VaR and expected shortfall assume the supplied observations are
representative. They do not independently address regime change, liquidity,
serial dependence or stressed scenarios.

## Reproducibility

- Random-number seeds are explicit and deterministic.
- Tests contain financial invariants as well as numeric reference values.
- CI builds Debug and Release configurations on Linux, macOS and Windows.
- A separate Linux job runs address and undefined-behaviour sanitizers.
- The project uses only the C++ standard library.

## Appropriate use

The library is an educational and portfolio implementation. It can be used to
study algorithms, compare numerical methods and run controlled experiments. It
has not been independently model-validated and must not be used for live
trading, regulatory capital, client valuation or production risk reporting.

