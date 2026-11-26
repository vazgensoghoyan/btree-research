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
bool BasicBTree<Key, K, Search, OptimizationFlag>::contains_impl(Key key) const {
    const Node* node = m_root.get();

    for (;;) {
        std::size_t index = lower_bound_in_node(*node, key);

        if (index < node->key_count && node->keys[index] == key)
            return true;

        if (node->is_leaf)
            break;

        node = node->children[index].get();

    }

    return false;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
std::optional<Key> BasicBTree<Key, K, Search, OptimizationFlag>::lower_bound_impl(Key key) const {
    const Node* node = m_root.get();
    std::optional<Key> lower = std::nullopt;

    for (;;) {
        std::size_t index = lower_bound_in_node(*node, key);

        if (index < node->key_count) {
            Key value = node->keys[index];

            if (value == key)
                return key;

            if (!lower.has_value() || value < lower.value())
                lower = value;
        }

        if (node->is_leaf)
            break;

        node = node->children[index].get();
    }

    return lower;
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
            insert_key_into_leaf(*node, index, key);
            ++m_size;
            break;
        }

        if (node->children[index]->is_full()) {
            split_full_node(*node, index);

            if constexpr (OptimizationFlag) {
                if (key == node->keys[index]) {
                    return false; // duplicate
                }
                if (key > node->keys[index]) {
                    ++index;
                }
            } else {
                continue; // try again
            }
        }

        node = node->children[index].get();
    }

    return true;
}


template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
bool BasicBTree<Key, K, Search, OptimizationFlag>::erase_impl(Key key) {
    Node* node = m_root.get();

    for (;;) {
        // possible after merge in previous iteration
        if (node == m_root.get() && !node->is_leaf && node->key_count == 0) {
            m_root = std::move(node->children[0]);
            node = m_root.get();
        }

        std::size_t index = lower_bound_in_node(*node, key);

        if (!(index < node->key_count && node->keys[index] == key)) { // !found

            if (node->is_leaf) {
                return false;
            }

            if (node->children[index]->is_low()) {
                unite_or_rotate(*node, index);
            } else {
                node = node->children[index].get();
            }

            continue;
        }

        if (node->is_leaf) {
            remove_key_from_leaf(*node, index);
            --m_size;
            return true;
        }

        if (!node->children[index]->is_low()) {
            node->keys[index] = extract_max(*node->children[index]);
            --m_size;
            return true;
        }

        if (!node->children[index + 1]->is_low()) {
            node->keys[index] = extract_min(*node->children[index + 1]);
            --m_size;
            return true;
        }

        unite_nodes(*node, index);
    }
}

// HELPERS

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
void BasicBTree<Key, K, Search, OptimizationFlag>::split_root_if_full() {
    if (!m_root->is_full()) {
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
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::insert_key_at(Node& node, std::size_t index, Key key) noexcept {
    assert(!node.is_full());
    assert(index <= node.key_count);

    for (std::size_t i = node.key_count; i > index; --i) {
        node.keys[i] = node.keys[i - 1];
    }

    node.keys[index] = key;
    node.key_count++;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::insert_key_into_leaf(Node& leaf, std::size_t index, Key key) noexcept {
    assert(leaf.is_leaf);
    insert_key_at(leaf, index, key);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ Key BasicBTree<Key, K, Search, OptimizationFlag>::remove_key_from_leaf(Node& leaf, std::size_t index) noexcept {
    assert(leaf.is_leaf);
    assert(index < leaf.key_count);

    const Key removed = leaf.keys[index];
    std::copy(leaf.keys + index + 1, leaf.keys + leaf.key_count, leaf.keys + index);
    --leaf.key_count;
    return removed;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::insert_in_node(Node& node, std::size_t index, Key key, std::unique_ptr<Node> right_child) noexcept {
    insert_key_at(node, index, key);
    std::move_backward(node.children + index + 1,
                       node.children + node.key_count,
                       node.children + node.key_count + 1);
    node.children[index + 1] = std::move(right_child);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::split_full_node(Node& parent, std::size_t child_index) {
    assert(child_index <= parent.key_count);
    assert(!parent.is_full());

    Node& child = *parent.children[child_index];
    assert(child.is_full());

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

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::unite_or_rotate(Node& parent, std::size_t child_index) noexcept {
    if (!parent.children[child_index]->is_low()) {
        return;
    }
    if (child_index == parent.key_count) {
        if (!parent.children[child_index - 1]->is_low()) {
            rotate_right(parent, child_index - 1);
        } else {
            unite_nodes(parent, child_index - 1);
        }
    } else {
        if (!parent.children[child_index + 1]->is_low()) {
            rotate_left(parent, child_index);
        } else {
            unite_nodes(parent, child_index);
        }
    }
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::unite_nodes(Node& parent, std::size_t key_index) noexcept {
    assert(key_index < parent.key_count);
    assert(parent.children[key_index]->is_low());
    assert(parent.children[key_index + 1]->is_low());

    Node& left = *parent.children[key_index];
    std::unique_ptr<Node> right = std::move(parent.children[key_index + 1]);
    assert(left.is_leaf == right->is_leaf); // let it be, just checking already guaranteed invariant

    left.keys[K - 1] = parent.keys[key_index];
    std::copy_n(right->keys, K - 1, left.keys + K);
    std::move(right->children, right->children + K, left.children + K);
    left.key_count = 2 * K - 1;

    std::copy(parent.keys + key_index + 1,
              parent.keys + parent.key_count,
              parent.keys + key_index);
    std::move(parent.children + key_index + 2,
              parent.children + parent.key_count + 1,
              parent.children + key_index + 1);
    --parent.key_count;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::rotate_left(Node& parent, std::size_t key_index) noexcept {
    assert(key_index < parent.key_count);
    assert(parent.children[key_index]->is_low());
    assert(!parent.children[key_index + 1]->is_low());

    Node& left = *parent.children[key_index];
    Node& right = *parent.children[key_index + 1];
    assert(left.is_leaf == right.is_leaf);

    left.keys[left.key_count] = parent.keys[key_index];
    left.children[left.key_count + 1] = std::move(right.children[0]);
    ++left.key_count;

    parent.keys[key_index] = right.keys[0];
    std::copy(right.keys + 1, right.keys + right.key_count, right.keys);
    std::move(right.children + 1,
              right.children + right.key_count + 1,
              right.children);
    --right.key_count;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ void BasicBTree<Key, K, Search, OptimizationFlag>::rotate_right(Node& parent, std::size_t key_index) noexcept {
    assert(key_index < parent.key_count);
    assert(!parent.children[key_index]->is_low());
    assert(parent.children[key_index + 1]->is_low());

    Node& left = *parent.children[key_index];
    Node& right = *parent.children[key_index + 1];
    assert(left.is_leaf == right.is_leaf);

    std::copy_backward(right.keys,
                       right.keys + right.key_count,
                       right.keys + right.key_count + 1);
    std::move_backward(right.children,
                       right.children + right.key_count + 1,
                       right.children + right.key_count + 2);
    right.keys[0] = parent.keys[key_index];
    right.children[0] = std::move(left.children[left.key_count]);
    ++right.key_count;

    parent.keys[key_index] = left.keys[left.key_count - 1];
    --left.key_count;
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ Key BasicBTree<Key, K, Search, OptimizationFlag>::extract_max(Node& subtree) noexcept {
    assert(!subtree.is_low());
    Node* node = &subtree;

    while (!node->is_leaf) {
        const std::size_t last = node->key_count;
        if (node->children[last]->is_low()) {
            unite_or_rotate(*node, last);
        } else {
            node = node->children[last].get();
        }
    }

    return remove_key_from_leaf(*node, node->key_count - 1);
}

template <BTreeKey Key, std::size_t K, search::LowerBoundSearch<Key> Search, bool OptimizationFlag>
/*static*/ Key BasicBTree<Key, K, Search, OptimizationFlag>::extract_min(Node& subtree) noexcept {
    assert(!subtree.is_low());
    Node* node = &subtree;

    while (!node->is_leaf) {
        if (node->children[0]->is_low()) {
            unite_or_rotate(*node, 0);
        } else {
            node = node->children[0].get();
        }
    }

    return remove_key_from_leaf(*node, 0);
}

} // namespace iheay
