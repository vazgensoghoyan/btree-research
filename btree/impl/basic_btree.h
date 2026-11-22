#pragma once

#include "btree/btree.h"

#include <cstddef>
#include <functional>

namespace iheay {

template <BTreeKey Key,
          std::size_t K = 4,
          class Compare = std::less<>>
class BasicBTree
    : public BTreeBase<
        BasicBTree<Key, K, Compare>,
        Key> {
    using Base = BTreeBase<
        BasicBTree<Key, K, Compare>,
        Key>;

    friend Base;

public:
    BasicBTree() = default;

private:
    std::size_t size_impl() const noexcept {
        return m_size;
    }

    bool contains_impl(Key key) const;

    bool insert_impl(Key key);

private:
    static constexpr std::size_t s_min_degree = K;
    static constexpr std::size_t s_max_degree = 2 * K;

private:
    struct Node {
        bool leaf;
        std::size_t key_count;
        Key keys[s_max_degree - 1];
        Node* children[s_max_degree];
    };

private:
    Node* m_root = nullptr;
    std::size_t m_size = 0;
    Compare m_compare{};
};

}
