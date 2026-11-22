#include <stdexcept>

namespace iheay {

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
    return false;
}

// HELPERS

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
    if (node.key_count >= s_max_degree - 1)
        throw std::logic_error("insert_key_at: node is full");
    if (index > node.key_count)
        throw std::out_of_range("insert_key_at: invalid index");

    for (std::size_t i = node.key_count; i > index; --i) {
        node.keys[i] = std::move(node.keys[i - 1]);
    }

    node.keys[index] = std::move(key);
    node.key_count++;
}

} // namespace iheay
