#pragma once

#include "search_concept.h"

#include <cstddef>
#include <span>

namespace iheay::search {

struct SimdSearch {
    template <class Key>
    constexpr std::size_t operator()(std::span<const Key> keys, const Key& key) const {
        return keys.size(); // just a stub
    }
};

static_assert(LowerBoundSearch<SimdSearch, int>);

} // namespace iheay::search
