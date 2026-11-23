#pragma once

#include "search_concept.h"

#include <cstddef>
#include <span>

namespace iheay::search {

struct BinarySearch {
    template <class Key>
    constexpr std::size_t operator()(std::span<const Key> keys, const Key& key) const {
        std::size_t left = 0;
        std::size_t right = keys.size();

        while (left < right) {
            std::size_t middle = (left + right) / 2;

            if (keys[middle] < key) {
                left = middle + 1;
            } else {
                right = middle;
            }
        }

        return left;
    }
};

static_assert(LowerBoundSearch<BinarySearch, int>);

} // namespace iheay::search
