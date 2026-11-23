#include "btree/impl/basic_btree.h"
#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <random>
#include <set>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

template <class Container, bool Hits>
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
        benchmark::DoNotOptimize(container.contains(queries[next]));
        next = (next + 1) & (queries.size() - 1);
    }
}

template <class Container>
void Register(const char* name) {
    const std::string prefix{name};
    auto* hits = benchmark::RegisterBenchmark((prefix + "/hit").c_str(),
                                               &Lookup<Container, true>);
    auto* misses = benchmark::RegisterBenchmark((prefix + "/miss").c_str(),
                                                 &Lookup<Container, false>);

    for (int count : {1'000, 10'000, 100'000}) {
        hits->Arg(count);
        misses->Arg(count);
    }
}

} // namespace

int main(int argc, char** argv) {
    Register<iheay::BasicBTree<int, 2, iheay::search::LinearSearch>>("btree_k2_linear");
    Register<iheay::BasicBTree<int, 2, iheay::search::BinarySearch>>("btree_k2_binary");
    Register<iheay::BasicBTree<int, 2, iheay::search::SimdSearch>>("btree_k2_simd");
    Register<iheay::BasicBTree<int, 4, iheay::search::LinearSearch>>("btree_k4_linear");
    Register<iheay::BasicBTree<int, 4, iheay::search::BinarySearch>>("btree_k4_binary");
    Register<iheay::BasicBTree<int, 4, iheay::search::SimdSearch>>("btree_k4_simd");
    Register<iheay::BasicBTree<int, 16, iheay::search::LinearSearch>>("btree_k16_linear");
    Register<iheay::BasicBTree<int, 16, iheay::search::BinarySearch>>("btree_k16_binary");
    Register<iheay::BasicBTree<int, 16, iheay::search::SimdSearch>>("btree_k16_simd");
    Register<std::set<int>>("std_set");
    Register<std::unordered_set<int>>("std_unordered_set");

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
