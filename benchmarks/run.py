#!/usr/bin/env python3
"""Build, run, and report the search and whole-tree benchmarks."""

import argparse
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
REPORT = ROOT / "benchmarks" / "report.py"
TARGETS = {"search": "b_tree_benchmarks", "tree": "whole_tree_benchmarks"}


def run(command: list[str]) -> None:
    print("+", " ".join(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--optimization", choices=("O0", "O1", "O2", "O3"),
                        default="O3", help="optimization level (default: O3)")
    parser.add_argument("--all-optimizations", action="store_true",
                        help="run O0, O1, O2, and O3")
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--min-time", default="0.1s")
    args = parser.parse_args()
    if args.repetitions < 1:
        parser.error("--repetitions must be positive")

    levels = ("O0", "O1", "O2", "O3") if args.all_optimizations else (args.optimization,)
    results = []

    for level in levels:
        build = ROOT / f"build-bench-{level.lower()}"
        configure = ["cmake", "-S", str(ROOT), "-B", str(build),
                     "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_CXX_FLAGS_RELEASE=-{level} -DNDEBUG",
                     "-DBUILD_TESTING=OFF", "-DBUILD_BENCHMARKS=ON"]
        cached_benchmark = ROOT / "build-benchmarks/_deps/googlebenchmark-src"
        if cached_benchmark.is_dir():
            configure.append(f"-DFETCHCONTENT_SOURCE_DIR_GOOGLEBENCHMARK={cached_benchmark}")
        run(configure)
        run(["cmake", "--build", str(build), "--target", *TARGETS.values()])

        for suite, target in TARGETS.items():
            output = build / f"{suite}.json"
            run([str(build / "benchmarks" / target),
                 f"--benchmark_repetitions={args.repetitions}",
                 f"--benchmark_min_time={args.min_time}",
                 f"--benchmark_out={output}", "--benchmark_out_format=json"])
            results.append(output)

    report = ROOT / "benchmarks" / "report.html"
    run([sys.executable, str(REPORT), *map(str, results), "-o", str(report)])
    print(f"Open {report.relative_to(ROOT)}", flush=True)


if __name__ == "__main__":
    main()
