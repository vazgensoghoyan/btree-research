#include <cassert>
#include <algorithm>

namespace iheay {

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
BasicBTree<Key, K, Search, OptimizationFlag>::BasicBTree()
    : m_root(std::make_unique<Node>(
        Node {
            .is_leaf = true,
        }))
    , m_size(0)
    , m_search({})
{}

// CRTP CONTRACT METHODS

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
std::size_t BasicBTree<Key, K, Search, OptimizationFlag>::size_impl() const noexcept {
    return m_size;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
bool BasicBTree<Key, K, Search, OptimizationFlag>::contains_impl(Key key) const {
    const Node* node = m_root.get();

    for (;;) {
        std::size_t index = lower_bound_in_node(*node, key);

        if (index < node->key_count &&
            node->keys[index] == key)
            return true;

        if (node->is_leaf)
            break;

        node = node->children[index].get();

    }

    return false;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
bool BasicBTree<Key, K, Search, OptimizationFlag>::insert_impl(Key key) {
    split_root_if_full();

    Node* node = m_root.get();

    for (;;) {
        std::size_t index = lower_bound_in_node(*node, key);

        if (index < node->key_count && node->keys[index] == key)
            return false; // duplicate
        
        if (node->is_leaf) { // curr node is the needed not full leaf
            insert_key_at(*node, index, key);
            ++m_size;
            break;
        }

        if (is_node_full(*node->children[index])) {
            split_full_node(*node, index);

            if (key == node->keys[index]) {
                return false; // duplicate
            }
            if (key > node->keys[index]) {
                ++index;
            }
        }

        node = node->children[index].get();
    }

    return true;
}

// HELPERS

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ bool BasicBTree<Key, K, Search, OptimizationFlag>::is_node_full(const Node& node) noexcept {
    return node.key_count + 1 == 2 * K; // degree == 2K
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
void BasicBTree<Key, K, Search, OptimizationFlag>::split_root_if_full() {
    if (!is_node_full(*m_root)) {
        return;
    }

    auto new_root = std::make_unique<Node>(Node {
        .is_leaf = false,
    });
    new_root->children[0] = std::move(m_root);
    m_root = std::move(new_root);

    split_full_node(*m_root, 0);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
std::size_t BasicBTree<Key, K, Search, OptimizationFlag>::lower_bound_in_node(const Node& node, Key key) const {
    std::span<const Key> keys_view{node.keys, node.key_count};
    return m_search(keys_view, key);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
void BasicBTree<Key, K, Search, OptimizationFlag>::insert_key_at(Node& node, std::size_t index, Key key) {
    assert(!is_node_full(node));
    assert(index <= node.key_count);

    for (std::size_t i = node.key_count; i > index; --i) {
        node.keys[i] = node.keys[i - 1];
    }

    node.keys[index] = key;
    node.key_count++;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
void BasicBTree<Key, K, Search, OptimizationFlag>::insert_in_node(Node& node, std::size_t index, Key key, std::unique_ptr<Node> right_child) {
    insert_key_at(node, index, key);
    std::move_backward(node.children + index + 1,
                       node.children + node.key_count,
                       node.children + node.key_count + 1);
    node.children[index + 1] = std::move(right_child);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
void BasicBTree<Key, K, Search, OptimizationFlag>::split_full_node(Node& parent, std::size_t child_index) {
    assert(child_index <= parent.key_count);
    assert(!is_node_full(parent));

    Node& child = *parent.children[child_index];
    assert(is_node_full(child));

    std::unique_ptr<Node> right_node = std::make_unique<Node>(Node {
        .is_leaf = child.is_leaf,
        .key_count = K - 1,
    });

    std::copy_n(child.keys + K, K - 1, right_node->keys);
    std::move(child.children + K,
              child.children + 2 * K,
              right_node->children);
    child.key_count = K - 1;

    insert_in_node(parent,
                   child_index,
                   child.keys[K - 1],
                   std::move(right_node));
}

} // namespace iheay
