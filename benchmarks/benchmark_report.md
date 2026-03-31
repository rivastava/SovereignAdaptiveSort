# SovereignAdaptiveSort Benchmark 2026-03-31

This benchmark covers the current `SovereignAdaptiveSort-v2` candidate using branchless leaf kernels, a type-specific two-stage dispatch probe, run-aware routing, integer stable-sort fallback, and `std::sort` fallback.

- Candidate: `SovereignAdaptiveSort-v2`
- Single-run all-win vs `std::sort`: `false`
- Single-run mean speedup vs `std::sort`: `1.7896x`
- Single-run worst family speedup vs `std::sort`: `0.9806x`
- Single-run best family speedup vs `std::sort`: `4.5539x`

## Single-Run Family Results

- `k_runs_float`: candidate `29.529` ns/elem, std::sort `31.604`, speedup `1.0703x`
- `k_runs_int`: candidate `24.689` ns/elem, std::sort `27.652`, speedup `1.1200x`
- `nearly_sorted_float`: candidate `9.701` ns/elem, std::sort `9.610`, speedup `0.9906x`
- `nearly_sorted_int`: candidate `11.849` ns/elem, std::sort `11.619`, speedup `0.9806x`
- `random_float`: candidate `23.967` ns/elem, std::sort `27.591`, speedup `1.1512x`
- `random_int`: candidate `13.670` ns/elem, std::sort `24.693`, speedup `1.8064x`
- `reverse_float`: candidate `0.649` ns/elem, std::sort `1.758`, speedup `2.7088x`
- `reverse_int`: candidate `0.547` ns/elem, std::sort `2.491`, speedup `4.5539x`
- `sorted_float`: candidate `0.724` ns/elem, std::sort `1.102`, speedup `1.5221x`
- `sorted_int`: candidate `0.508` ns/elem, std::sort `1.012`, speedup `1.9921x`

## Honest Read

- The strongest current single-run result still misses one weak family, so this remains a near-universal candidate rather than a proven all-family win.
- Repeat-run stability should still be checked alongside this single-run table before making strong public claims.
