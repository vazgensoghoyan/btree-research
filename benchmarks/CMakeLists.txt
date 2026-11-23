# Benchmark dependency.
find_package(benchmark CONFIG QUIET)

if (NOT benchmark_FOUND)
	include(FetchContent)

	set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
	set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
	set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(
		googlebenchmark
		GIT_REPOSITORY https://github.com/google/benchmark.git
		GIT_TAG v1.9.4
	)
	FetchContent_MakeAvailable(googlebenchmark)
endif()

# Benchmark executable.
add_executable(b_tree_benchmarks example_benchmark.cpp)

target_compile_features(b_tree_benchmarks PRIVATE cxx_std_20)
target_link_libraries(b_tree_benchmarks PRIVATE benchmark::benchmark_main)
