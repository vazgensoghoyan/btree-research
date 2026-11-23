#pragma once

#include "btree/btree.h"

#include "btree/search/search_concept.h"
#include "btree/search/linear_search.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <span>

namespace iheay {

/*
 * Invariants of our BasicBTree
 *
 * - Keys in each node are strictly increasing
 * - Every leaf is at the same depth
 *
 * - for root: 0 <= node.key_count <= 2*K - 1
 * - for other nodes: K-1 <= node.key_count <= 2*K - 1
 *
 * - any node except root has 1 parent
 * - by degree I mean child count:
 *
 * - for leaves: degree == 0
 * - for other nodes: degree == key_count + 1
 *
 * - For any separator key[i], keys in child[i] are smaller, while keys in child[i + 1] are greater.
 */

template <BTreeKey Key,
          std::size_t K = 8,
          search::LowerBoundSearch<Key> Search = search::LinearSearch>
class BasicBTree
    : public BTreeBase<BasicBTree<Key, K, Search>, Key> {

    using Base = BTreeBase<BasicBTree<Key, K, Search>,Key>;

    friend Base;

public:
    BasicBTree();

    BasicBTree(const BasicBTree&) = delete;
    BasicBTree& operator=(const BasicBTree&) = delete;
    BasicBTree(BasicBTree&&) = delete;
    BasicBTree& operator=(BasicBTree&&) = delete;

    static_assert(K >= 2, "B-tree minimum degree must be at least 2");

private:
    struct Node {
        bool is_leaf = true;
        std::size_t key_count = 0;
        Key keys[2 * K - 1] = {};
        std::unique_ptr<Node> children[2 * K] = {nullptr};
    };

private:
    // for CRTP contract in 'btree/btree.h'
    std::size_t size_impl() const noexcept;
    bool contains_impl(Key key) const;
    bool insert_impl(Key key);

    // helpers
    static bool is_node_full(const Node& node) noexcept;
    std::size_t lower_bound_in_node(const Node& node, Key key) const;
    void insert_key_at(Node& node, std::size_t index, Key key);
    void insert_in_node(Node& node, std::size_t index, Key key, std::unique_ptr<Node> right_child);
    void split_full_node(Node& parent, std::size_t child_index);

private:
    std::unique_ptr<Node> m_root;
    std::size_t m_size;
    Search m_search;
};

} // namespace iheay

#include "basic_btree.tpp"
