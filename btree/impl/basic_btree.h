#pragma once

#include "btree/btree.h"

#include "btree/search/search_concept.h"
#include "btree/search/linear_search.h"

#include <cstddef>
#include <functional>
#include <memory>

namespace iheay {

template <BTreeKey Key,
          std::size_t K = 4,
          search::LowerBoundSearch<Key> Search = search::LinearSearch>
class BasicBTree
    : public BTreeBase<BasicBTree<Key, K, Search>, Key> {

    using Base = BTreeBase<BasicBTree<Key, K, Search>,Key>;

    friend Base;

public:
    BasicBTree();

private:
    struct Node {
        bool is_leaf = true;
        // key_count + 1 == degree
        // for root: 0 <= degree <= 2*K
        // for other nodes: K <= degree <= 2*K
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
    bool is_node_full(Node& node);
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
