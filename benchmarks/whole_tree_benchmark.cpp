#include "btree/impl/basic_btree.h"

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
    Register<iheay::BasicBTree<int, 4>>("btree_k4");
    Register<iheay::BasicBTree<int, 16>>("btree_k16");
    Register<iheay::BasicBTree<int, 64>>("btree_k64");
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
