#include <benchmark/benchmark.h>

namespace {

void Example(benchmark::State& state) {
    for (auto _ : state) {
        int value = 1 + 1;
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(Example);

}  // namespace
