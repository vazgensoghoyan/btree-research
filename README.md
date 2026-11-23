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

Configure a separate Release build with benchmarks enabled, then build and run
the benchmark executable:

```sh
cmake -S . -B build-benchmarks \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DBUILD_BENCHMARKS=ON
cmake --build build-benchmarks --target b_tree_benchmarks
./build-benchmarks/benchmarks/b_tree_benchmarks
```

Google Benchmark is found as an installed package when available; otherwise,
CMake fetches Google Benchmark v1.9.4. Run selected benchmarks or save results
as JSON with:

```sh
./build-benchmarks/benchmarks/b_tree_benchmarks --benchmark_filter=Example
./build-benchmarks/benchmarks/b_tree_benchmarks \
  --benchmark_out=benchmark-results.json --benchmark_out_format=json
```
