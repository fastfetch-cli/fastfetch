# Fastfetch Benchmark Data

This branch stores continuous performance benchmark data for Fastfetch on Linux, macOS, and Windows.

The data is generated and updated automatically by GitHub Actions. Please do not edit generated benchmark files manually.

## Benchmark Dashboard

<https://fastfetch-cli.github.io/fastfetch/dev/bench/>

## Metrics

The benchmark data includes:

- Execution time for each Fastfetch module, measured in milliseconds;
- Total Fastfetch process execution time, measured in milliseconds.

All metrics use the “smaller is better” comparison mode.

## Update Workflow

Benchmark data is generated from the `dev` branch CI workflow and updated after all platform build jobs have completed.

Related workflow files:

- `.github/workflows/build-benchmark.yml`
- `.github/workflows/ci.yml`

Commit information and dates are recorded automatically by the Continuous Benchmark Action.
