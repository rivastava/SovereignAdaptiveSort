# SovereignAdaptiveSort

`SovereignAdaptiveSort` is an experimental C++ hybrid sorter that combines:

- branchless fixed-size leaf kernels for very small segments
- cheap input sampling to detect strong order structure
- a run-aware merge path for strongly structured arrays
- conservative fallbacks to `std::sort` and `std::stable_sort`

## Package Layout

- `SovereignAdaptiveSort_v2.cpp`
  Standalone reference implementation and benchmark driver.
- `benchmarks/benchmark_report.md`
  Human-readable summary of the current benchmark run.
- `benchmarks/benchmark.json`
  Machine-readable benchmark summary.
- `benchmarks/benchmark.tsv`
  Raw benchmark table.
- `benchmarks/stability.json`
  Repeat-run stability summary for near-parity families.

## Build

```bash
clang++ -std=c++20 -O3 -o SovereignAdaptiveSort_v2 SovereignAdaptiveSort_v2.cpp
```

## Current Result Snapshot

- best single-run result is a near-universal candidate rather than a strict all-family winner
- strongest wins are on sorted, reverse-sorted, and multi-run inputs
- weakest families sit close to parity, so repeat-run stability matters

## Notes

- This folder is intended to stand alone as an adaptive-sort release.
- Benchmark artifacts here are limited to the adaptive-sort candidate and its repeat-run stability summary.
