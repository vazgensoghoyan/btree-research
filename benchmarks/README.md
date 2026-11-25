# Benchmarks

Run from the repository root:

```sh
python3 benchmarks/scripts/run.py
```

The script configures Release builds at `-O2` and `-O3`, builds both benchmark
executables, runs every case with three repetitions, saves raw JSON in the
corresponding `build-bench-o2/` and `build-bench-o3/` directories, and
writes [results/report.html](results/report.html). Open that file directly in a browser. It is a
standalone page, so it also works from a GitHub checkout. Commit the generated
file when you want others to download or open the latest results. GitHub's
repository file viewer shows HTML source; an interactive web link requires
hosting the file, for example with GitHub Pages.

Use `python3 benchmarks/scripts/run.py --optimization O2` or `--optimization O3`
to run one selected level. `--repetitions 5 --min-time 0.5s` requests longer
measurements. The script uses the same `-DNDEBUG` setting for both levels.

The report has **Node search** and **Whole-tree lookup** views. Select the
build and hit/miss workload, then use **Chart** or **Table** to inspect median
CPU nanoseconds per lookup and individual runs. Lower is faster. Raw JSON can
be regenerated into a report without benchmarking again:

```sh
python3 benchmarks/scripts/report.py build-bench-o3/search.json build-bench-o3/tree.json \
  -o benchmarks/results/report.html
```

## What is measured

**Node search** compares linear, binary, SIMD, and `std::lower_bound` searches
on sorted `int` arrays of 4 to 128 keys. Each iteration searches for one query.

**Whole-tree lookup** compares `BasicBTree<int, K, Search>` with `K` of 2, 4,
and 16, using linear, binary, and SIMD node search, plus `std::set<int>`.
Each container is filled
with the same shuffled even keys before timing. The timed operation is one
`contains` call. Hit queries are present keys; miss queries are odd numbers.
Tree sizes are 1,000, 10,000, and 100,000 keys.

Both suites use reproducible query sequences.
The benchmark targets compile with `-mavx2` and require an AVX2 CPU. These
measurements are specific to the
machine, compiler, flags, and workload. Small differences deserve repeated runs
under stable CPU conditions.

When adding another benchmark executable, register its target in `TARGETS` in
`scripts/run.py` and teach `scripts/report.py` how to group its result names.
