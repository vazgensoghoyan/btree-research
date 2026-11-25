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
          std::size_t K = 16,
          search::LowerBoundSearch<Key> Search = search::LinearSearch,
          bool OptimizationFlag = false>
class BasicBTree
    : public BTreeBase<BasicBTree<Key, K, Search, OptimizationFlag>, Key> {

    using Base = BTreeBase<BasicBTree<Key, K, Search, OptimizationFlag>,Key>;

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

        bool is_full() const noexcept { return key_count == 2 * K - 1; }
        bool is_low() const noexcept { return key_count == K - 1; }
    };

private:
    // for CRTP contract in 'btree/btree.h'

    std::size_t size_impl() const noexcept { return m_size; }
    bool contains_impl(Key key) const;
    bool insert_impl(Key key); // false -> duplicate
    bool erase_impl(Key key); // false -> key absent

    // helpers

    void split_root_if_full();

    std::size_t lower_bound_in_node(const Node& node, Key key) const;

    static void insert_key_into_leaf(Node& leaf, std::size_t index, Key key) noexcept;
    static Key remove_key_from_leaf(Node& leaf, std::size_t index) noexcept;

    static void insert_key_at(Node& node, std::size_t index, Key key) noexcept;
    static void insert_in_node(Node& node, std::size_t index, Key key, std::unique_ptr<Node> right_child) noexcept;

    static void split_full_node(Node& parent, std::size_t child_index);

    static void unite_or_rotate(Node& parent, std::size_t child_index) noexcept;
    static void unite_nodes(Node& parent, std::size_t key_index) noexcept;

    static void rotate_left(Node& parent, std::size_t key_index) noexcept;
    static void rotate_right(Node& parent, std::size_t key_index) noexcept;

    static Key extract_max(Node& subtree) noexcept;
    static Key extract_min(Node& subtree) noexcept;

private:
    std::unique_ptr<Node> m_root;
    std::size_t m_size;
    Search m_search;
};

} // namespace iheay

#include "basic_btree.tpp"
