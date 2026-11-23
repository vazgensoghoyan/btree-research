#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <vector>

namespace {

struct StdLowerBound {
    std::size_t operator()(std::span<const int> keys, int query) const {
        return static_cast<std::size_t>(
            std::lower_bound(keys.begin(), keys.end(), query) - keys.begin());
    }
};

// All engines receive the same keys and query sequence for each case.
template <class Engine, bool Hits>
void Search(benchmark::State& state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    std::vector<int> keys(count);
    std::iota(keys.begin(), keys.end(), 0);
    for (int& key : keys) {
        key *= 2;
    }

    std::mt19937 generator{0x5EED};
    std::uniform_int_distribution<std::size_t> position{0, count - 1};
    std::vector<int> queries(4096);
    for (int& query : queries) {
        const int base = keys[position(generator)];
        // Hits use stored keys; misses fall between consecutive keys.
        query = Hits ? base : base + 1;
    }

    const std::span<const int> view{keys};
    const Engine engine{};
    std::size_t next = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(engine(view, queries[next]));
        next = (next + 1) & (queries.size() - 1);
    }
}

template <class Engine>
void Register(const char* name) {
    const std::string prefix{name};
    auto* hits = benchmark::RegisterBenchmark((prefix + "/hit").c_str(),
                                               &Search<Engine, true>);
    auto* misses = benchmark::RegisterBenchmark((prefix + "/miss").c_str(),
                                                 &Search<Engine, false>);

    for (int count : {4, 8, 16, 32, 64, 128}) {
        hits->Arg(count);
        misses->Arg(count);
    }
}

} // namespace

int main(int argc, char** argv) {
    Register<iheay::search::LinearSearch>("linear");
    Register<iheay::search::BinarySearch>("binary");
    Register<iheay::search::SimdSearch>("simd");
    Register<StdLowerBound>("std_lower_bound");

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
