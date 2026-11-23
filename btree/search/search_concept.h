#pragma once

#include <concepts>
#include <cstddef>
#include <span>

namespace iheay::search {

/*
 * A search engine accepts sorted keys and returns the insertion position
 * of key: the first index whose value is not less than key.
 *
 * Returning keys.size() represents a query greater than every key.
 */

template <class Engine, class Key>
concept LowerBoundSearch =
    requires(const Engine& engine, std::span<const Key> keys, const Key& key) {
        { engine(keys, key) } -> std::same_as<std::size_t>;
    };

} // namespace iheay::search
