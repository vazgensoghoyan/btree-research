#include "btree/impl/basic_btree.h"
#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <span>
#include <string>
#include <vector>

namespace {

template <class Container>
std::optional<int> LowerBound(const Container& container, int key) {
    return container.lower_bound(key);
}

std::optional<int> LowerBound(const std::set<int>& container, int key) {
    const auto found = container.lower_bound(key);
    if (found == container.end()) {
        return std::nullopt;
    }
    return *found;
}

enum class LookupOperation { Contains, LowerBound };

template <class Container, bool Hits,
          LookupOperation Operation = LookupOperation::Contains>
void Lookup(benchmark::State& state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    std::vector<int> keys(count);
    std::iota(keys.begin(), keys.end(), 0);
    for (int& key : keys) {
        key *= 2;
    }

    std::mt19937 generator{0x5EED};
    std::shuffle(keys.begin(), keys.end(), generator);

    Container container;
    if constexpr (requires(Container& value) { value.reserve(count); }) {
        container.reserve(count);
    }
    for (int key : keys) {
        container.insert(key);
    }

    std::uniform_int_distribution<std::size_t> position{0, count - 1};
    std::vector<int> queries(4096);
    for (int& query : queries) {
        const int key = keys[position(generator)];
        query = Hits ? key : key + 1;
    }

    std::size_t next = 0;
    for (auto _ : state) {
        if constexpr (Operation == LookupOperation::LowerBound) {
            auto result = LowerBound(container, queries[next]);
            benchmark::DoNotOptimize(result);
        } else {
            benchmark::DoNotOptimize(container.contains(queries[next]));
        }
        next = (next + 1) & (queries.size() - 1);
    }
}

template <class Container>
void Insert(benchmark::State& state) {
    const auto count = static_cast<std::size_t>(state.range(0));
    std::vector<int> keys(count);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 generator{0x5EED};
    std::shuffle(keys.begin(), keys.end(), generator);

    for (auto _ : state) {
        state.PauseTiming();
        {
            Container container;
            state.ResumeTiming();
            for (int key : keys) {
                container.insert(key);
            }
            benchmark::DoNotOptimize(container.size());
            state.PauseTiming();
        }
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * count);
}

template <class Container>
void Register(const char* name) {
    const std::string prefix{name};
    auto* hits = benchmark::RegisterBenchmark((prefix + "/hit").c_str(),
                                               &Lookup<Container, true>);
    auto* misses = benchmark::RegisterBenchmark((prefix + "/miss").c_str(),
                                                 &Lookup<Container, false>);
    auto* inserts = benchmark::RegisterBenchmark((prefix + "/insert").c_str(),
                                                  &Insert<Container>);
    auto* lower_bound_hits = benchmark::RegisterBenchmark(
        (prefix + "/lower_bound_hit").c_str(),
        &Lookup<Container, true, LookupOperation::LowerBound>);
    auto* lower_bound_misses = benchmark::RegisterBenchmark(
        (prefix + "/lower_bound_miss").c_str(),
        &Lookup<Container, false, LookupOperation::LowerBound>);

    for (int count : {1'000, 10'000, 100'000}) {
        hits->Arg(count);
        misses->Arg(count);
        inserts->Arg(count);
        lower_bound_hits->Arg(count);
        lower_bound_misses->Arg(count);
    }
}

template <std::size_t K, class Search>
void RegisterTree(const char* name) {
    const std::string prefix{name};
    Register<iheay::BasicBTree<int, K, Search, false>>((prefix + "_opt_off").c_str());
    Register<iheay::BasicBTree<int, K, Search, true>>((prefix + "_opt_on").c_str());
}

} // namespace

int main(int argc, char** argv) {
    RegisterTree<2, iheay::search::LinearSearch>("btree_k2_linear");
    RegisterTree<2, iheay::search::BinarySearch>("btree_k2_binary");
    RegisterTree<2, iheay::search::SimdSearch>("btree_k2_simd");
    RegisterTree<4, iheay::search::LinearSearch>("btree_k4_linear");
    RegisterTree<4, iheay::search::BinarySearch>("btree_k4_binary");
    RegisterTree<4, iheay::search::SimdSearch>("btree_k4_simd");
    RegisterTree<16, iheay::search::LinearSearch>("btree_k16_linear");
    RegisterTree<16, iheay::search::BinarySearch>("btree_k16_binary");
    RegisterTree<16, iheay::search::SimdSearch>("btree_k16_simd");
    Register<std::set<int>>("std_set");

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
