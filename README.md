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

The suite compares lower-bound search engines and whole-tree lookups and insertions across
several B-tree node capacities and `std::set`.
See [benchmarks/README.md](benchmarks/README.md) for the workload, interactive
HTML report, result comparisons, and compiler optimization experiments.

Build, run, and generate the browsable report with one command:

```sh
python3 benchmarks/scripts/run.py
```

Open [benchmarks/results/report.html](benchmarks/results/report.html) for the
generated chart and table. The runner measures both `-O2` and `-O3` by default.
