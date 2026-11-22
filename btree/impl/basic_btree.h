#pragma once

#include "btree/btree.h"

#include <cstddef>
#include <functional>
#include <memory>

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
    static constexpr std::size_t s_min_degree = K;
    static constexpr std::size_t s_max_degree = 2 * K;

    struct Node {
        bool leaf;
        std::size_t key_count;
        Key keys[s_max_degree - 1];
        std::unique_ptr<Node> children[s_max_degree];
    };

private:
    // for CRTP contract in 'btree/btree.h'
    std::size_t size_impl() const noexcept;
    bool contains_impl(Key key) const;
    bool insert_impl(Key key);

    // helpers
    std::size_t lower_bound_in_node(const Node& node, Key key) const;
    void insert_key_at(Node& node, std::size_t index, Key key);

private:
    std::unique_ptr<Node> m_root = nullptr;
    std::size_t m_size = 0;
    Compare m_compare{};
};

} // namespace iheay

#include "basic_btree.tpp"
