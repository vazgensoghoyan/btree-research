#pragma once

#include "search_concept.h"
#include "linear_search.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <bit>
#include <type_traits>

#include <immintrin.h>

namespace iheay::search {

struct SimdSearch {
    template <class Key>
    std::size_t operator()(std::span<const Key> keys, const Key& key) const {
        if constexpr (std::is_same_v<Key, int>) {
            if (__builtin_cpu_supports("avx2"))
                return search_avx2_int(keys, key);
        }
        return LinearSearch{}(keys, key);
    }

private:
    std::size_t search_avx2_int(std::span<const int> keys, int key) const {
        __m256i target_vector = _mm256_set1_epi32(key);
        __m256i key_vector;

        std::size_t lower_bound = 0;
        std::size_t i = 0;

        for (; i + 7 < keys.size(); i += 8) {
            key_vector = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&keys[i]));

            __m256i comparison = _mm256_cmpgt_epi32(target_vector, key_vector);
            __m256 casted_comparison = _mm256_castsi256_ps(comparison);
            int cmp_mask = _mm256_movemask_ps(casted_comparison);

            lower_bound += std::popcount(static_cast<uint32_t>(cmp_mask));

            if (!(cmp_mask & 0x80)) // after that all bigger
                return lower_bound;
        }

        for (; i < keys.size(); i += 1) {
            if (keys[i] >= key) break;
            ++lower_bound;
        }

        return lower_bound;
    }
};

static_assert(LowerBoundSearch<SimdSearch, int>);

} // namespace iheay::search
