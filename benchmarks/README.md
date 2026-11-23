# Search benchmarks

`b_tree_benchmarks` currently measures lower-bound lookup within a sorted array.
It compares `LinearSearch`, `BinarySearch`, `SimdSearch`, and `std::lower_bound`.
It does **not** measure B-tree traversal or insertion yet.

Each engine gets the same sorted `int` keys and the same 4,096 queries. Cases
use 4, 8, 16, 32, 64, or 128 keys. Keys are even numbers; hit queries are
present keys, while miss queries are odd numbers. A fixed random seed chooses
query positions. Setup happens before timing; each timed iteration performs one
lookup and consumes its result. Names have the form `engine/hit-or-miss/key-count`,
for example `simd/miss/32`.

`SimdSearch` currently uses AVX2 for `int`; run these benchmarks on a CPU with
AVX2 support. The benchmark executable uses Google Benchmark's CPU time by
default. Results are nanoseconds per lookup when the table reports `ns`.

## Run and save results

From the repository root:

```sh
cmake -S . -B build-benchmarks \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DBUILD_BENCHMARKS=ON
cmake --build build-benchmarks --target b_tree_benchmarks
./build-benchmarks/benchmarks/b_tree_benchmarks --benchmark_list_tests
./build-benchmarks/benchmarks/b_tree_benchmarks \
  --benchmark_repetitions=5 \
  --benchmark_out=build-benchmarks/results.json \
  --benchmark_out_format=json
```

To run a subset, add a filter such as `--benchmark_filter='simd/(hit|miss)/32$'`.
Google Benchmark prints each run and summary statistics for repeated runs. In
the JSON, `cpu_time` is the measured time per iteration and `time_unit` gives
its unit. Look at the median and spread across repetitions, not just one run.

To compare two saved JSON files, use Google Benchmark's comparison tool:

```sh
python3 build-benchmarks/_deps/googlebenchmark-src/tools/compare.py benchmarks \
  build-benchmarks/baseline.json build-benchmarks/contender.json
```

The comparison pairs cases by benchmark name. Keep the workload, key count,
compiler, and machine the same when interpreting a speed ratio. For comparisons
across optimization levels, record the flags and label each result file.

## Compare optimization levels

Use a separate build directory for each level. For example:

```sh
cmake -S . -B build-bench-o2 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE='-O2 -DNDEBUG' \
  -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=ON
cmake --build build-bench-o2 --target b_tree_benchmarks
./build-bench-o2/benchmarks/b_tree_benchmarks \
  --benchmark_repetitions=5 \
  --benchmark_out=build-bench-o2/results.json \
  --benchmark_out_format=json
```

Repeat with `-O0`, `-O1`, or `-O3`, changing the directory name and flag together.
Setting `CMAKE_CXX_FLAGS_RELEASE` explicitly avoids mixing CMake's default
Release optimization flag with the requested one. Keep `-DNDEBUG` consistent
across builds. The JSON files can be compared with the command above.

Run benchmarks on an otherwise idle machine and repeat them if differences are
small. These timings include query selection and loop overhead, which matter
most for the shortest arrays. They show relative search costs under this
workload, not whole-tree performance.
