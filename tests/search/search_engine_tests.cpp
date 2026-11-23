#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <array>
#include <cstddef>
#include <span>
#include <tuple>

#include <gtest/gtest.h>

namespace {

using SearchEngines = ::testing::Types<
    iheay::search::LinearSearch,
    iheay::search::BinarySearch,
    iheay::search::SimdSearch>;

template <class Engine>
class SearchEngineTest : public ::testing::Test {};

TYPED_TEST_SUITE(SearchEngineTest, SearchEngines);

template <class Engine, class Key, std::size_t N>
void ExpectLowerBounds(const Engine& engine,
                       const std::array<Key, N>& keys,
                       const std::array<std::tuple<Key, std::size_t>, N * 2 + 1>& cases) {
    for (const auto& [query, expected] : cases) {
        EXPECT_EQ(engine(std::span<const Key>{keys}, query), expected)
            << "query=" << query;
    }
}

TYPED_TEST(SearchEngineTest, EmptyInputReturnsZero) {
    constexpr std::array<int, 0> keys{};
    EXPECT_EQ(TypeParam{}(std::span<const int>{keys}, 42), 0U);
}

TYPED_TEST(SearchEngineTest, FindsLowerBoundAcrossSortedKeys) {
    constexpr std::array keys{1, 3, 5, 7};
    constexpr std::array cases{
        std::tuple{0, std::size_t{0}},
        std::tuple{1, std::size_t{0}},
        std::tuple{2, std::size_t{1}},
        std::tuple{3, std::size_t{1}},
        std::tuple{4, std::size_t{2}},
        std::tuple{6, std::size_t{3}},
        std::tuple{7, std::size_t{3}},
        std::tuple{8, std::size_t{4}},
        std::tuple{9, std::size_t{4}}
    };

    ExpectLowerBounds(TypeParam{}, keys, cases);
}

TYPED_TEST(SearchEngineTest, DuplicateKeysReturnFirstEqualPosition) {
    constexpr std::array keys{1, 3, 3, 3, 8};
    constexpr std::array cases{
        std::tuple{3, std::size_t{1}},
        std::tuple{4, std::size_t{4}}
    };

    for (const auto& [query, expected] : cases) {
        EXPECT_EQ(TypeParam{}(std::span<const int>{keys}, query), expected);
    }
}

} // namespace
