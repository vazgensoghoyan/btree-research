#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace iheay {

template <class T>
concept BTreeKey =
    std::is_arithmetic_v<T> &&
    std::is_trivially_copyable_v<T> &&
    (sizeof(T) <= 8);

template <class Impl, BTreeKey Key>
class BTreeBase {
public:
    bool empty() const noexcept {
        return size() == 0;
    }

    std::size_t size() const noexcept {
        return impl().size_impl();
    }

    bool contains(Key key) const {
        return impl().contains_impl(std::move(key));
    }

    bool insert(Key key) {
        return impl().insert_impl(std::move(key));
    }

private:
    Impl& impl() noexcept {
        return static_cast<Impl&>(*this);
    }

    const Impl& impl() const noexcept {
        return static_cast<const Impl&>(*this);
    }
};

}
