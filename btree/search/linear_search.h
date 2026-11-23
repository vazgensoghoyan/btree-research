#pragma once

#include "search_concept.h"

#include <cstddef>
#include <span>

namespace iheay::search {

struct LinearSearch {
    template <class Key>
    constexpr std::size_t operator()(std::span<const Key> keys, const Key& key) const {
        std::size_t index = 0;
        while (index < keys.size() && keys[index] < key) {
            ++index;
        }
        return index;
    }
};

static_assert(LowerBoundSearch<LinearSearch, int>);

} // namespace iheay::search
