#include <iostream>
#include <vector>
#include <cstddef>

#pragma GCC target("avx2")

#include <bits/stdc++.h>
#include <x86intrin.h>

#include "btree/search/binary_search.h"

int main() {
    // Trying Binary Search

    std::vector<int32_t> nums = { 1, 3, 4, 4, 4, 6, 7, 9, 9 };

    iheay::search::BinarySearch engine{};

    std::span<const int32_t> nums_view{nums};
    int32_t key = 4;

    std::size_t index = engine(nums_view, key);

    std::cout << index << std::endl;

    // SIMD

    std::cout << std::endl;
    std::cout << __builtin_cpu_supports("sse") << std::endl;
    std::cout << __builtin_cpu_supports("sse2") << std::endl;
    std::cout << __builtin_cpu_supports("avx") << std::endl;
    std::cout << __builtin_cpu_supports("avx2") << std::endl;
    std::cout << __builtin_cpu_supports("avx512f") << std::endl;
    std::cout << std::endl;

    alignas(32) int keys[9] = {10, 20, 30, 40, 30, 60, -17, 13};
    int target = 25;

    __m256i key_vector = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(keys));
    __m256i target_vector = _mm256_set1_epi32(target);

    __m256i comparison = _mm256_cmpgt_epi32(target_vector, key_vector);
    __m256 casted_comparison = _mm256_castsi256_ps(comparison);
    int cmp_mask = _mm256_movemask_ps(casted_comparison);

    std::cout << "Mask : ";
    for (int i = 0; i < 8; ++i) {
        std::cout << ((cmp_mask >> i) & 1);
    }
    std::cout << std::endl;

    //

    return 0;
}
