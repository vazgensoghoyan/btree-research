#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <array>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <random>
#include <span>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

namespace {

using SearchEngines = ::testing::Types<
    iheay::search::LinearSearch,
    iheay::search::BinarySearch,
    iheay::search::SimdSearch>;

template <class Engine>
class SearchEngineTest : public ::testing::Test {};

TYPED_TEST_SUITE(SearchEngineTest, SearchEngines);

template <class Engine, class Key, std::size_t N, std::size_t M>
void ExpectLowerBounds(const Engine& engine,
                       const std::array<Key, N>& keys,
                       const std::array<std::tuple<Key, std::size_t>, M>& cases) {
    for (const auto& [query, expected] : cases) {
        EXPECT_EQ(engine(std::span<const Key>{keys}, query), expected)
            << "query=" << query << ", key_count=" << keys.size();
    }
}

TYPED_TEST(SearchEngineTest, EmptyInputReturnsZero) {
    constexpr std::array<int, 0> keys{};
    EXPECT_EQ(TypeParam{}(std::span<const int>{keys}, 42), 0U);
}

TYPED_TEST(SearchEngineTest, HandlesShortInput) {
    constexpr std::array<int, 4> keys{10, 20, 30, 40};
    constexpr std::array cases{
        std::tuple{5, std::size_t{0}},
        std::tuple{10, std::size_t{0}},
        std::tuple{25, std::size_t{2}},
        std::tuple{40, std::size_t{3}},
        std::tuple{50, std::size_t{4}}
    };
    ExpectLowerBounds(TypeParam{}, keys, cases);
}

TYPED_TEST(SearchEngineTest, HandlesOneFullVector) {
    constexpr std::array<int, 8> keys{10, 20, 30, 40, 50, 60, 70, 80};
    constexpr std::array cases{
        std::tuple{5, std::size_t{0}},
        std::tuple{10, std::size_t{0}},
        std::tuple{15, std::size_t{1}},
        std::tuple{20, std::size_t{1}},
        std::tuple{75, std::size_t{7}},
        std::tuple{80, std::size_t{7}},
        std::tuple{85, std::size_t{8}}
    };
    ExpectLowerBounds(TypeParam{}, keys, cases);
}

TYPED_TEST(SearchEngineTest, HandlesVectorAndScalarTail) {
    constexpr std::array<int, 11> keys{10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110};
    constexpr std::array cases{
        std::tuple{75, std::size_t{7}},
        std::tuple{80, std::size_t{7}},
        std::tuple{85, std::size_t{8}},
        std::tuple{90, std::size_t{8}},
        std::tuple{95, std::size_t{9}},
        std::tuple{110, std::size_t{10}},
        std::tuple{115, std::size_t{11}}
    };
    ExpectLowerBounds(TypeParam{}, keys, cases);
}

TYPED_TEST(SearchEngineTest, DuplicateKeysReturnFirstEqualPosition) {
    constexpr std::array<int, 12> keys{1, 3, 3, 3, 8, 10, 12, 14, 16, 18, 20, 22};
    constexpr std::array cases{
        std::tuple{3, std::size_t{1}},
        std::tuple{4, std::size_t{4}},
        std::tuple{14, std::size_t{7}},
        std::tuple{15, std::size_t{8}},
        std::tuple{23, std::size_t{12}}
    };
    ExpectLowerBounds(TypeParam{}, keys, cases);
}

TYPED_TEST(SearchEngineTest, MatchesStdLowerBoundOnReproducibleRandomInputs) {
    std::mt19937 generator{0x5EED};
    std::uniform_int_distribution<int> value{-50, 50};
    constexpr std::array<std::size_t, 15> sizes{
        0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33, 64, 127};

    for (std::size_t size : sizes) {
        for (int sample = 0; sample < 40; ++sample) {
            std::vector<int> keys(size);
            for (int& key : keys) {
                key = value(generator);
            }
            std::sort(keys.begin(), keys.end());

            const std::span<const int> view{keys};
            const auto check_query = [&](int query) {
                const auto expected = static_cast<std::size_t>(
                    std::lower_bound(keys.begin(), keys.end(), query) - keys.begin());
                EXPECT_EQ(TypeParam{}(view, query), expected)
                    << "size=" << size << ", sample=" << sample << ", query=" << query;
            };

            check_query(std::numeric_limits<int>::min());
            check_query(std::numeric_limits<int>::max());
            for (int key : keys) {
                check_query(key);
            }
            for (int query = 0; query < 30; ++query) {
                check_query(value(generator));
            }
        }
    }
}

} // namespace
