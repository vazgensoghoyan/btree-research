#include <cassert>
#include <algorithm>

namespace iheay {

template <BTreeKey Key, std::size_t K, class Compare>
BasicBTree<Key, K, Compare>::BasicBTree()
    : m_root(std::make_unique<Node>(
        Node {
            .is_leaf = true,
        }))
    , m_size(0)
    , m_compare({})
{}

// CRTP CONTRACT METHODS

template <BTreeKey Key, std::size_t K, class Compare>
std::size_t BasicBTree<Key, K, Compare>::size_impl() const noexcept {
    return m_size;
}

template <BTreeKey Key, std::size_t K, class Compare>
bool BasicBTree<Key, K, Compare>::contains_impl(Key key) const {
    return false;
}

template <BTreeKey Key, std::size_t K, class Compare>
bool BasicBTree<Key, K, Compare>::insert_impl(Key key) {
    if (is_node_full(*m_root)) {
        std::unique_ptr<Node> new_root = std::make_unique<Node>(Node {
            .is_leaf = static_cast<bool>(m_root),
            .key_count = 0,
        });
        new_root->children[0] = std::move(m_root);
        m_root = std::move(new_root);

        split_full_node(*m_root, 0);
    }

    Node* curr_node = m_root.get();

    while (!curr_node->is_leaf) {
        std::size_t child_index = lower_bound_in_node(*curr_node, key);
        auto& child_ptr = curr_node->children[child_index];

        if (is_node_full(*child_ptr)) {
            split_full_node(*curr_node, child_index);
            continue; // try again
        }

        curr_node = child_ptr.get();
    }

    // curr node is the needed not full leaf
    std::size_t key_index = lower_bound_in_node(*curr_node, key);
    insert_key_at(*curr_node, key_index, key);

    ++m_size;

    return true;
}

// HELPERS

template <BTreeKey Key, std::size_t K, class Compare>
bool BasicBTree<Key, K, Compare>::is_node_full(Node& node) {
    return node.key_count + 1 == 2 * K; // degree == 2K
}

template <BTreeKey Key, std::size_t K, class Compare>
std::size_t BasicBTree<Key, K, Compare>::lower_bound_in_node(const Node& node, Key key) const {
    for (std::size_t i = 0; i < node.key_count; ++i) {
        if (!m_compare(node.keys[i], key)) { // key <= node->keys[i]
            return i;
        }
    }
    return node.key_count;
}

template <BTreeKey Key, std::size_t K, class Compare>
void BasicBTree<Key, K, Compare>::insert_key_at(Node& node, std::size_t index, Key key) {
    assert(!is_node_full(node));
    assert(index <= node.key_count);

    for (std::size_t i = node.key_count; i > index; --i) {
        node.keys[i] = node.keys[i - 1];
    }

    node.keys[index] = key;
    node.key_count++;
}

template <BTreeKey Key, std::size_t K, class Compare>
void BasicBTree<Key, K, Compare>::insert_in_node(Node& node, std::size_t index, Key key, std::unique_ptr<Node> right_child) {
    insert_key_at(node, index, key);
    std::move_backward(node.children + index + 1,
                       node.children + node.key_count,
                       node.children + node.key_count + 1);
    node.children[index + 1] = std::move(right_child);
}

template <BTreeKey Key, std::size_t K, class Compare>
void BasicBTree<Key, K, Compare>::split_full_node(Node& parent, std::size_t child_index) {
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
