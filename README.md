# B-tree practice

Configure, build, and run the tests:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The test target uses an installed GoogleTest package when available. Otherwise,
CMake fetches GoogleTest v1.17.0. Pass `-DBUILD_TESTING=OFF` during configuration
to build the playground without tests.

## Benchmarks

The current suite compares lower-bound search engines on sorted `int` arrays.
See [benchmarks/README.md](benchmarks/README.md) for the workload, result format,
comparison commands, and compiler optimization experiments.

Build and run all cases:

```sh
cmake -S . -B build-benchmarks \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DBUILD_BENCHMARKS=ON
cmake --build build-benchmarks --target b_tree_benchmarks
./build-benchmarks/benchmarks/b_tree_benchmarks
```

Google Benchmark is found as an installed package when available; otherwise,
CMake fetches Google Benchmark v1.9.4.

```sh
./build-benchmarks/benchmarks/b_tree_benchmarks --benchmark_list_tests
./build-benchmarks/benchmarks/b_tree_benchmarks \
  --benchmark_repetitions=5 \
  --benchmark_out=build-benchmarks/results.json --benchmark_out_format=json
```
