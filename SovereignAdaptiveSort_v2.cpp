#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

template <typename T>
inline void compare_swap(T& a, T& b) noexcept {
    T lo = std::min(a, b);
    T hi = std::max(a, b);
    a = lo;
    b = hi;
}

template <typename T>
inline void leaf_sort_2(std::span<T, 2> x) noexcept {
    compare_swap(x[0], x[1]);
}

template <typename T>
inline void leaf_sort_3(std::span<T, 3> x) noexcept {
    compare_swap(x[0], x[1]);
    compare_swap(x[1], x[2]);
    compare_swap(x[0], x[1]);
}

template <typename T>
inline void leaf_sort_4(std::span<T, 4> x) noexcept {
    compare_swap(x[0], x[1]); compare_swap(x[2], x[3]);
    compare_swap(x[0], x[2]); compare_swap(x[1], x[3]);
    compare_swap(x[1], x[2]);
}

template <typename T>
inline void leaf_sort_8(std::span<T, 8> x) noexcept {
    compare_swap(x[0], x[1]); compare_swap(x[2], x[3]); compare_swap(x[4], x[5]); compare_swap(x[6], x[7]);
    compare_swap(x[0], x[2]); compare_swap(x[1], x[3]); compare_swap(x[4], x[6]); compare_swap(x[5], x[7]);
    compare_swap(x[1], x[2]); compare_swap(x[5], x[6]); compare_swap(x[0], x[4]); compare_swap(x[1], x[5]);
    compare_swap(x[2], x[6]); compare_swap(x[3], x[7]); compare_swap(x[2], x[4]); compare_swap(x[3], x[5]);
    compare_swap(x[1], x[2]); compare_swap(x[3], x[4]); compare_swap(x[5], x[6]);
}

template <typename T>
inline void leaf_sort_16(std::span<T, 16> x) noexcept {
    compare_swap(x[0],  x[1]);  compare_swap(x[2],  x[3]);  compare_swap(x[4],  x[5]);  compare_swap(x[6],  x[7]);
    compare_swap(x[8],  x[9]);  compare_swap(x[10], x[11]); compare_swap(x[12], x[13]); compare_swap(x[14], x[15]);
    compare_swap(x[0],  x[2]);  compare_swap(x[1],  x[3]);  compare_swap(x[4],  x[6]);  compare_swap(x[5],  x[7]);
    compare_swap(x[8],  x[10]); compare_swap(x[9],  x[11]); compare_swap(x[12], x[14]); compare_swap(x[13], x[15]);
    compare_swap(x[0],  x[4]);  compare_swap(x[1],  x[5]);  compare_swap(x[2],  x[6]);  compare_swap(x[3],  x[7]);
    compare_swap(x[8],  x[12]); compare_swap(x[9],  x[13]); compare_swap(x[10], x[14]); compare_swap(x[11], x[15]);
    compare_swap(x[0],  x[8]);  compare_swap(x[1],  x[9]);  compare_swap(x[2],  x[10]); compare_swap(x[3],  x[11]);
    compare_swap(x[4],  x[12]); compare_swap(x[5],  x[13]); compare_swap(x[6],  x[14]); compare_swap(x[7],  x[15]);
    compare_swap(x[5],  x[10]); compare_swap(x[6],  x[9]);  compare_swap(x[3],  x[12]); compare_swap(x[13], x[14]);
    compare_swap(x[7],  x[11]); compare_swap(x[1],  x[2]);  compare_swap(x[4],  x[8]);
    compare_swap(x[1],  x[4]);  compare_swap(x[7],  x[13]); compare_swap(x[2],  x[8]);  compare_swap(x[11], x[14]);
    compare_swap(x[5],  x[6]);  compare_swap(x[9],  x[10]);
    compare_swap(x[2],  x[4]);  compare_swap(x[11], x[13]); compare_swap(x[3],  x[8]);  compare_swap(x[7],  x[12]);
    compare_swap(x[6],  x[8]);  compare_swap(x[10], x[12]); compare_swap(x[3],  x[5]);  compare_swap(x[7],  x[9]);
    compare_swap(x[3],  x[4]);  compare_swap(x[5],  x[6]);  compare_swap(x[7],  x[8]);  compare_swap(x[9],  x[10]);
    compare_swap(x[11], x[12]);
    compare_swap(x[6],  x[7]);  compare_swap(x[8],  x[9]);
}

constexpr std::size_t kLeafThreshold = 16;
constexpr std::size_t kMinRun = 32;
constexpr double kMaxRunRatio = 0.03;
constexpr std::size_t kIntQuickSamplePairs = 4;
constexpr std::size_t kFloatQuickSamplePairs = 2;
constexpr std::size_t kFullSamplePairs = 64;
constexpr double kIntQuickRunMinRatio = 0.50;
constexpr double kIntQuickRunMaxRatio = 0.68;
constexpr double kIntQuickRunMaxInvRatio = 0.40;
constexpr std::size_t kIntQuickRunMaxDirectionChanges = 3;
constexpr double kFloatQuickRunMinRatio = 0.60;
constexpr double kFloatQuickRunMaxRatio = 0.75;
constexpr double kFloatQuickRunMaxInvRatio = 0.30;
constexpr std::size_t kFloatQuickRunMaxDirectionChanges = 1;
constexpr double kRunGateMinRatio = 0.12;
constexpr double kRunGateMaxRatio = 0.28;
constexpr double kRunGateMaxInvRatio = 0.20;
constexpr std::size_t kRunGateMaxDirectionChanges = 18;
constexpr double kIntStableSortMinInvRatio = 0.35;

template <typename T>
constexpr std::size_t quick_sample_pairs_for() {
    if constexpr (std::is_integral_v<T>) {
        return kIntQuickSamplePairs;
    }
    return kFloatQuickSamplePairs;
}

template <typename T>
constexpr double quick_run_min_ratio_for() {
    if constexpr (std::is_integral_v<T>) {
        return kIntQuickRunMinRatio;
    }
    return kFloatQuickRunMinRatio;
}

template <typename T>
constexpr double quick_run_max_ratio_for() {
    if constexpr (std::is_integral_v<T>) {
        return kIntQuickRunMaxRatio;
    }
    return kFloatQuickRunMaxRatio;
}

template <typename T>
constexpr double quick_run_max_inv_ratio_for() {
    if constexpr (std::is_integral_v<T>) {
        return kIntQuickRunMaxInvRatio;
    }
    return kFloatQuickRunMaxInvRatio;
}

template <typename T>
constexpr std::size_t quick_run_max_direction_changes_for() {
    if constexpr (std::is_integral_v<T>) {
        return kIntQuickRunMaxDirectionChanges;
    }
    return kFloatQuickRunMaxDirectionChanges;
}

template <typename T>
void insertion_sort(T* data, std::size_t n) {
    for (std::size_t i = 1; i < n; ++i) {
        T key = data[i];
        std::size_t j = i;
        while (j > 0 && key < data[j - 1]) {
            data[j] = data[j - 1];
            --j;
        }
        data[j] = key;
    }
}

template <typename T>
void sort_small(T* data, std::size_t n) {
    switch (n) {
        case 0:
        case 1:
            return;
        case 2:
            leaf_sort_2(std::span<T, 2>(data, 2));
            return;
        case 3:
            leaf_sort_3(std::span<T, 3>(data, 3));
            return;
        case 4:
            leaf_sort_4(std::span<T, 4>(data, 4));
            return;
        case 8:
            leaf_sort_8(std::span<T, 8>(data, 8));
            return;
        case 16:
            leaf_sort_16(std::span<T, 16>(data, 16));
            return;
        default:
            insertion_sort(data, n);
            return;
    }
}

template <typename T>
struct Run {
    std::size_t start;
    std::size_t end;
};

template <typename T>
std::vector<Run<T>> detect_runs(std::vector<T>& values) {
    std::vector<Run<T>> runs;
    const std::size_t n = values.size();
    if (n == 0) {
        return runs;
    }
    std::size_t start = 0;
    while (start < n) {
        std::size_t end = start + 1;
        if (end == n) {
            runs.push_back({start, end});
            break;
        }
        bool descending = values[end] < values[end - 1];
        while (end < n) {
            if (descending) {
                if (!(values[end] < values[end - 1])) {
                    break;
                }
            } else {
                if (values[end] < values[end - 1]) {
                    break;
                }
            }
            ++end;
        }
        if (descending) {
            std::reverse(values.begin() + static_cast<std::ptrdiff_t>(start),
                         values.begin() + static_cast<std::ptrdiff_t>(end));
        }
        runs.push_back({start, end});
        start = end;
    }
    return runs;
}

template <typename T>
std::vector<Run<T>> boost_runs(std::vector<T>& values, std::vector<Run<T>> runs, std::size_t minrun) {
    if (runs.empty()) {
        return runs;
    }
    std::vector<Run<T>> boosted;
    boosted.reserve(runs.size());
    std::size_t idx = 0;
    while (idx < runs.size()) {
        std::size_t start = runs[idx].start;
        std::size_t end = runs[idx].end;
        while ((end - start) < minrun && idx + 1 < runs.size()) {
            ++idx;
            end = runs[idx].end;
        }
        sort_small(values.data() + start, end - start);
        boosted.push_back({start, end});
        ++idx;
    }
    if (!boosted.empty() && boosted.back().end != values.size()) {
        boosted.back().end = values.size();
        sort_small(values.data() + boosted.back().start, boosted.back().end - boosted.back().start);
    }
    return boosted;
}

template <typename T>
void merge_two_runs(
    const std::vector<T>& src,
    std::vector<T>& dst,
    std::size_t left_start,
    std::size_t left_end,
    std::size_t right_end
) {
    std::size_t i = left_start;
    std::size_t j = left_end;
    std::size_t out = left_start;
    while (i < left_end && j < right_end) {
        if (src[j] < src[i]) {
            dst[out++] = src[j++];
        } else {
            dst[out++] = src[i++];
        }
    }
    while (i < left_end) {
        dst[out++] = src[i++];
    }
    while (j < right_end) {
        dst[out++] = src[j++];
    }
}

template <typename T>
void adaptive_run_sort(std::vector<T>& values) {
    if (values.size() <= kLeafThreshold) {
        sort_small(values.data(), values.size());
        return;
    }
    std::vector<Run<T>> runs = detect_runs(values);
    if (runs.size() <= 1) {
        return;
    }
    if (static_cast<double>(runs.size()) / static_cast<double>(values.size()) > kMaxRunRatio) {
        std::sort(values.begin(), values.end());
        return;
    }
    runs = boost_runs(values, std::move(runs), kMinRun);
    std::vector<T> buffer(values.size());
    std::vector<T>* src = &values;
    std::vector<T>* dst = &buffer;
    while (runs.size() > 1) {
        std::vector<Run<T>> merged;
        merged.reserve((runs.size() + 1) / 2);
        for (std::size_t i = 0; i < runs.size(); i += 2) {
            if (i + 1 >= runs.size()) {
                std::copy(src->begin() + static_cast<std::ptrdiff_t>(runs[i].start),
                          src->begin() + static_cast<std::ptrdiff_t>(runs[i].end),
                          dst->begin() + static_cast<std::ptrdiff_t>(runs[i].start));
                merged.push_back(runs[i]);
                continue;
            }
            const auto& left = runs[i];
            const auto& right = runs[i + 1];
            if (!((*src)[right.start] < (*src)[left.end - 1])) {
                std::copy(src->begin() + static_cast<std::ptrdiff_t>(left.start),
                          src->begin() + static_cast<std::ptrdiff_t>(right.end),
                          dst->begin() + static_cast<std::ptrdiff_t>(left.start));
            } else {
                merge_two_runs(*src, *dst, left.start, left.end, right.end);
            }
            merged.push_back({left.start, right.end});
        }
        std::swap(src, dst);
        runs = std::move(merged);
    }
    if (src != &values) {
        values.swap(*src);
    }
}

template <typename T>
bool is_sorted_exact(const std::vector<T>& values) {
    for (std::size_t i = 1; i < values.size(); ++i) {
        if (values[i] < values[i - 1]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool is_reverse_sorted_exact(const std::vector<T>& values) {
    for (std::size_t i = 1; i < values.size(); ++i) {
        if (values[i] > values[i - 1]) {
            return false;
        }
    }
    return true;
}

template <typename T>
struct SampleStats {
    bool all_non_decreasing = true;
    bool all_non_increasing = true;
    std::size_t direction_changes = 0;
    std::size_t effective_pairs = 0;
    std::size_t sampled_inversions = 0;
};

template <typename T>
SampleStats<T> sample_stats(const std::vector<T>& values, std::size_t sample_pairs) {
    SampleStats<T> stats;
    if (values.size() <= 2) {
        return stats;
    }
    const std::size_t pairs = std::min<std::size_t>(sample_pairs, values.size() - 1);
    const std::size_t step = std::max<std::size_t>(1, (values.size() - 1) / pairs);
    int last_sign = 0;
    for (std::size_t idx = step; idx < values.size(); idx += step) {
        const std::size_t prev = idx - step;
        int sign = 0;
        if (values[idx] < values[prev]) {
            sign = -1;
            ++stats.sampled_inversions;
            stats.all_non_decreasing = false;
        } else if (values[idx] > values[prev]) {
            sign = 1;
            stats.all_non_increasing = false;
        }
        if (sign != 0) {
            if (last_sign != 0 && sign != last_sign) {
                ++stats.direction_changes;
            }
            last_sign = sign;
        }
        ++stats.effective_pairs;
    }
    return stats;
}

template <typename T>
double sampled_run_ratio(const SampleStats<T>& stats) {
    return static_cast<double>(1 + stats.direction_changes) /
           static_cast<double>(std::max<std::size_t>(1, stats.effective_pairs));
}

template <typename T>
double sampled_inversion_ratio(const SampleStats<T>& stats) {
    return static_cast<double>(stats.sampled_inversions) /
           static_cast<double>(std::max<std::size_t>(1, stats.effective_pairs));
}

template <typename T>
void adaptive_dispatch_sort(std::vector<T>& values) {
    if (values.size() <= kLeafThreshold) {
        sort_small(values.data(), values.size());
        return;
    }
    const auto quick = sample_stats(values, quick_sample_pairs_for<T>());
    if (quick.all_non_decreasing && is_sorted_exact(values)) {
        return;
    }
    if (quick.all_non_increasing && is_reverse_sorted_exact(values)) {
        std::reverse(values.begin(), values.end());
        return;
    }
    const double quick_run_ratio = sampled_run_ratio(quick);
    const double quick_inv_ratio = sampled_inversion_ratio(quick);

    if constexpr (std::is_integral_v<T>) {
        if (quick_inv_ratio >= kIntStableSortMinInvRatio) {
            std::stable_sort(values.begin(), values.end());
            return;
        }
    }

    const bool maybe_run_structured =
        quick.direction_changes <= quick_run_max_direction_changes_for<T>() &&
        quick_run_ratio >= quick_run_min_ratio_for<T>() &&
        quick_run_ratio <= quick_run_max_ratio_for<T>() &&
        quick_inv_ratio <= quick_run_max_inv_ratio_for<T>();

    if (!maybe_run_structured) {
        std::sort(values.begin(), values.end());
        return;
    }

    const auto stats = sample_stats(values, kFullSamplePairs);
    const double run_ratio = sampled_run_ratio(stats);
    const double inv_ratio = sampled_inversion_ratio(stats);
    const bool strong_run_signature =
        stats.direction_changes <= kRunGateMaxDirectionChanges &&
        run_ratio >= kRunGateMinRatio &&
        run_ratio <= kRunGateMaxRatio &&
        inv_ratio <= kRunGateMaxInvRatio;

    if (strong_run_signature) {
        adaptive_run_sort(values);
        return;
    }

    std::sort(values.begin(), values.end());
}

std::vector<std::int32_t> gen_sorted_int(std::size_t n, std::mt19937& rng) {
    const std::int32_t start = static_cast<std::int32_t>(rng() % 2000) - 1000;
    const std::int32_t step = static_cast<std::int32_t>(1 + (rng() % 5));
    std::vector<std::int32_t> out(n);
    for (std::size_t i = 0; i < n; ++i) {
        out[i] = start + step * static_cast<std::int32_t>(i);
    }
    return out;
}

std::vector<std::int32_t> gen_reverse_int(std::size_t n, std::mt19937& rng) {
    auto out = gen_sorted_int(n, rng);
    std::reverse(out.begin(), out.end());
    return out;
}

std::vector<std::int32_t> gen_nearly_sorted_int(std::size_t n, std::mt19937& rng) {
    auto out = gen_sorted_int(n, rng);
    const std::size_t swaps = std::max<std::size_t>(1, n / 50);
    for (std::size_t rep = 0; rep < swaps; ++rep) {
        std::size_t a = rng() % n;
        std::size_t b = rng() % n;
        std::swap(out[a], out[b]);
    }
    return out;
}

std::vector<std::int32_t> gen_random_int(std::size_t n, std::mt19937& rng) {
    std::uniform_int_distribution<std::int32_t> dist(
        std::numeric_limits<std::int32_t>::min() / 2,
        std::numeric_limits<std::int32_t>::max() / 2
    );
    std::vector<std::int32_t> out(n);
    for (auto& value : out) {
        value = dist(rng);
    }
    return out;
}

std::vector<std::int32_t> gen_k_runs_int(std::size_t n, std::mt19937& rng) {
    const std::size_t k = std::min<std::size_t>(12, std::max<std::size_t>(2, n / 1024 + 2));
    std::vector<std::size_t> cuts;
    while (cuts.size() + 1 < k) {
        cuts.push_back(1 + (rng() % (n - 1)));
    }
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    cuts.push_back(n);
    std::vector<std::vector<std::int32_t>> blocks;
    std::size_t start = 0;
    std::int32_t offset = 0;
    for (std::size_t cut : cuts) {
        std::vector<std::int32_t> block(cut - start);
        for (auto& value : block) {
            value = offset + static_cast<std::int32_t>(rng() % static_cast<unsigned>(n / 2 + 8));
        }
        std::sort(block.begin(), block.end());
        offset += static_cast<std::int32_t>(rng() % 21);
        blocks.push_back(std::move(block));
        start = cut;
    }
    std::shuffle(blocks.begin(), blocks.end(), rng);
    std::vector<std::int32_t> out;
    out.reserve(n);
    for (const auto& block : blocks) {
        out.insert(out.end(), block.begin(), block.end());
    }
    return out;
}

std::vector<float> gen_sorted_float(std::size_t n, std::mt19937& rng) {
    std::normal_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> out(n);
    float acc = 0.0f;
    for (auto& value : out) {
        acc += std::abs(dist(rng)) + 0.01f;
        value = acc;
    }
    return out;
}

std::vector<float> gen_reverse_float(std::size_t n, std::mt19937& rng) {
    auto out = gen_sorted_float(n, rng);
    std::reverse(out.begin(), out.end());
    return out;
}

std::vector<float> gen_nearly_sorted_float(std::size_t n, std::mt19937& rng) {
    auto out = gen_sorted_float(n, rng);
    const std::size_t swaps = std::max<std::size_t>(1, n / 64);
    for (std::size_t rep = 0; rep < swaps; ++rep) {
        std::size_t a = rng() % n;
        std::size_t b = rng() % n;
        std::swap(out[a], out[b]);
    }
    return out;
}

std::vector<float> gen_random_float(std::size_t n, std::mt19937& rng) {
    std::normal_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> out(n);
    for (auto& value : out) {
        value = dist(rng);
    }
    return out;
}

std::vector<float> gen_k_runs_float(std::size_t n, std::mt19937& rng) {
    const std::size_t k = std::min<std::size_t>(12, std::max<std::size_t>(2, n / 1024 + 2));
    std::vector<std::size_t> cuts;
    while (cuts.size() + 1 < k) {
        cuts.push_back(1 + (rng() % (n - 1)));
    }
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    cuts.push_back(n);
    std::normal_distribution<float> dist(0.0f, 1.0f);
    std::vector<std::vector<float>> blocks;
    std::size_t start = 0;
    float offset = 0.0f;
    for (std::size_t cut : cuts) {
        std::vector<float> block(cut - start);
        for (auto& value : block) {
            value = offset + dist(rng);
        }
        std::sort(block.begin(), block.end());
        offset += 0.5f + static_cast<float>(rng() % 100) / 100.0f;
        blocks.push_back(std::move(block));
        start = cut;
    }
    std::shuffle(blocks.begin(), blocks.end(), rng);
    std::vector<float> out;
    out.reserve(n);
    for (const auto& block : blocks) {
        out.insert(out.end(), block.begin(), block.end());
    }
    return out;
}

struct ResultRow {
    std::string algorithm;
    std::string family;
    std::string kind;
    std::size_t arrays = 0;
    std::size_t total_n = 0;
    double total_ns = 0.0;
    bool correct = true;
};

template <typename T, typename Generator>
void benchmark_family(const std::string& family, const std::string& kind, Generator generator, std::vector<ResultRow>& rows) {
    std::mt19937 rng(20260331 + static_cast<unsigned>(family.size() * 29 + kind.size() * 11));
    const std::vector<std::size_t> sizes = {512, 2048, 8192, 32768, 131072};
    const int replicates = 4;

    ResultRow row_std{"std::sort", family, kind};
    ResultRow row_stable{"std::stable_sort", family, kind};
    ResultRow row_universal{"SovereignAdaptiveSort", family, kind};

    for (std::size_t n : sizes) {
        for (int rep = 0; rep < replicates; ++rep) {
            std::vector<T> base = generator(n, rng);
            std::vector<T> expected = base;
            std::sort(expected.begin(), expected.end());

            auto a = base;
            auto t0 = std::chrono::high_resolution_clock::now();
            std::sort(a.begin(), a.end());
            auto t1 = std::chrono::high_resolution_clock::now();
            row_std.total_ns += std::chrono::duration<double, std::nano>(t1 - t0).count();
            row_std.correct = row_std.correct && (a == expected);

            auto b = base;
            t0 = std::chrono::high_resolution_clock::now();
            std::stable_sort(b.begin(), b.end());
            t1 = std::chrono::high_resolution_clock::now();
            row_stable.total_ns += std::chrono::duration<double, std::nano>(t1 - t0).count();
            row_stable.correct = row_stable.correct && (b == expected);

            auto c = base;
            t0 = std::chrono::high_resolution_clock::now();
            adaptive_dispatch_sort(c);
            t1 = std::chrono::high_resolution_clock::now();
            row_universal.total_ns += std::chrono::duration<double, std::nano>(t1 - t0).count();
            row_universal.correct = row_universal.correct && (c == expected);

            row_std.arrays++;
            row_std.total_n += n;
            row_stable.arrays++;
            row_stable.total_n += n;
            row_universal.arrays++;
            row_universal.total_n += n;
        }
    }

    rows.push_back(row_std);
    rows.push_back(row_stable);
    rows.push_back(row_universal);
}

void print_results(const std::vector<ResultRow>& rows) {
    std::cout << "META\tbenchmark\tSovereignAdaptiveSort-v2\n";
    std::cout << "META\tleaf_threshold\t" << kLeafThreshold << "\n";
    std::cout << "META\tminrun\t" << kMinRun << "\n";
    std::cout << "META\tmax_run_ratio\t" << kMaxRunRatio << "\n";
    std::cout << "META\tint_quick_sample_pairs\t" << kIntQuickSamplePairs << "\n";
    std::cout << "META\tfloat_quick_sample_pairs\t" << kFloatQuickSamplePairs << "\n";
    std::cout << "META\tfull_sample_pairs\t" << kFullSamplePairs << "\n";
    std::cout << "META\tint_quick_run_min_ratio\t" << kIntQuickRunMinRatio << "\n";
    std::cout << "META\tint_quick_run_max_ratio\t" << kIntQuickRunMaxRatio << "\n";
    std::cout << "META\tint_quick_run_max_inv_ratio\t" << kIntQuickRunMaxInvRatio << "\n";
    std::cout << "META\tint_quick_run_max_direction_changes\t" << kIntQuickRunMaxDirectionChanges << "\n";
    std::cout << "META\tfloat_quick_run_min_ratio\t" << kFloatQuickRunMinRatio << "\n";
    std::cout << "META\tfloat_quick_run_max_ratio\t" << kFloatQuickRunMaxRatio << "\n";
    std::cout << "META\tfloat_quick_run_max_inv_ratio\t" << kFloatQuickRunMaxInvRatio << "\n";
    std::cout << "META\tfloat_quick_run_max_direction_changes\t" << kFloatQuickRunMaxDirectionChanges << "\n";
    std::cout << "META\trun_gate_min_ratio\t" << kRunGateMinRatio << "\n";
    std::cout << "META\trun_gate_max_ratio\t" << kRunGateMaxRatio << "\n";
    std::cout << "META\trun_gate_max_inv_ratio\t" << kRunGateMaxInvRatio << "\n";
    std::cout << "META\trun_gate_max_direction_changes\t" << kRunGateMaxDirectionChanges << "\n";
    std::cout << "META\tint_stable_sort_min_inv_ratio\t" << kIntStableSortMinInvRatio << "\n";
    for (const auto& row : rows) {
        const double ns_per_array = row.total_ns / static_cast<double>(row.arrays);
        const double ns_per_element = row.total_ns / static_cast<double>(row.total_n);
        std::cout << "RESULT"
                  << '\t' << row.algorithm
                  << '\t' << row.family
                  << '\t' << row.kind
                  << '\t' << row.arrays
                  << '\t' << row.total_n
                  << '\t' << std::fixed << std::setprecision(3) << ns_per_array
                  << '\t' << std::fixed << std::setprecision(3) << ns_per_element
                  << '\t' << (row.correct ? 1 : 0)
                  << '\n';
    }
}

}  // namespace

int main() {
    std::vector<ResultRow> rows;
    benchmark_family<std::int32_t>("sorted_int", "int", gen_sorted_int, rows);
    benchmark_family<std::int32_t>("reverse_int", "int", gen_reverse_int, rows);
    benchmark_family<std::int32_t>("nearly_sorted_int", "int", gen_nearly_sorted_int, rows);
    benchmark_family<std::int32_t>("random_int", "int", gen_random_int, rows);
    benchmark_family<std::int32_t>("k_runs_int", "int", gen_k_runs_int, rows);
    benchmark_family<float>("sorted_float", "float", gen_sorted_float, rows);
    benchmark_family<float>("reverse_float", "float", gen_reverse_float, rows);
    benchmark_family<float>("nearly_sorted_float", "float", gen_nearly_sorted_float, rows);
    benchmark_family<float>("random_float", "float", gen_random_float, rows);
    benchmark_family<float>("k_runs_float", "float", gen_k_runs_float, rows);
    print_results(rows);
    return 0;
}
