#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace iheay {

template <class T>
concept BTreeKey =
    std::is_integral_v<T> &&
    std::is_trivially_copyable_v<T> &&
    (sizeof(T) <= 8);

template <class Impl, BTreeKey Key>
class BTreeBase {
public:
    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return impl().size_impl();
    }

    [[nodiscard]] bool contains(Key key) const {
        return impl().contains_impl(key);
    }

    bool insert(Key key) {
        return impl().insert_impl(key);
    }

    bool erase(Key key) {
        return impl().erase_impl(key);
    }

private:
    Impl& impl() noexcept {
        return static_cast<Impl&>(*this);
    }

    const Impl& impl() const noexcept {
        return static_cast<const Impl&>(*this);
    }
};

} // namespace iheay
