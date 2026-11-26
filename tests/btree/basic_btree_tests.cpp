#include "btree/impl/basic_btree.h"
#include "btree/search/binary_search.h"
#include "btree/search/linear_search.h"
#include "btree/search/simd_search.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <vector>

namespace {

using BTreeTypes = ::testing::Types<
    iheay::BasicBTree<int, 2>,
    iheay::BasicBTree<int, 2, iheay::search::BinarySearch>,
    iheay::BasicBTree<int, 2, iheay::search::SimdSearch>,
    iheay::BasicBTree<int, 4>,
    iheay::BasicBTree<int, 4, iheay::search::BinarySearch>,
    iheay::BasicBTree<int, 4, iheay::search::SimdSearch>,
    iheay::BasicBTree<int, 16>,
    iheay::BasicBTree<int, 16, iheay::search::BinarySearch>,
    iheay::BasicBTree<int, 16, iheay::search::SimdSearch>>;

template <class Tree>
class BasicBTreeTest : public ::testing::Test {
protected:
    Tree tree;
};

TYPED_TEST_SUITE(BasicBTreeTest, BTreeTypes);

template <class Tree>
void ExpectMatches(const Tree& tree, const std::set<int>& expected) {
    ASSERT_EQ(tree.size(), expected.size());
    ASSERT_EQ(tree.empty(), expected.empty());

    for (int key = -5; key <= 205; ++key) {
        EXPECT_EQ(tree.contains(key), expected.contains(key)) << "key=" << key;
    }
}

template <class Tree>
void InsertAll(Tree& tree, std::set<int>& expected, const std::vector<int>& keys) {
    for (int key : keys) {
        const bool inserted = expected.insert(key).second;
        ASSERT_EQ(tree.insert(key), inserted) << "key=" << key;
    }
}

template <class Tree>
void ExpectLowerBounds(const Tree& tree, const std::set<int>& expected, int first, int last) {
    for (int key = first; key <= last; ++key) {
        const auto it = expected.lower_bound(key);
        const std::optional<int> result = it == expected.end()
            ? std::nullopt : std::optional<int>(*it);
        EXPECT_EQ(tree.lower_bound(key), result) << "key=" << key;
    }
}

TYPED_TEST(BasicBTreeTest, StartsEmpty) {
    EXPECT_TRUE(this->tree.empty());
    EXPECT_EQ(this->tree.size(), 0U);
    EXPECT_EQ(this->tree.lower_bound(0), std::nullopt);
}

TYPED_TEST(BasicBTreeTest, LowerBoundHandlesSingleKey) {
    ASSERT_TRUE(this->tree.insert(42));
    EXPECT_EQ(this->tree.lower_bound(41), 42);
    EXPECT_EQ(this->tree.lower_bound(42), 42);
    EXPECT_EQ(this->tree.lower_bound(43), std::nullopt);
}

TYPED_TEST(BasicBTreeTest, LowerBoundMatchesSetAcrossMultipleLevels) {
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    for (int& key : keys) key = key * 2 - 100;
    std::mt19937 random(20261126);
    std::shuffle(keys.begin(), keys.end(), random);
    InsertAll(this->tree, expected, keys);

    const TypeParam& tree = this->tree;
    ExpectLowerBounds(tree, expected, -102, 300);

    std::shuffle(keys.begin(), keys.end(), random);
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(this->tree.erase(keys[i]));
        expected.erase(keys[i]);
    }
    ExpectLowerBounds(tree, expected, -102, 300);
}

TYPED_TEST(BasicBTreeTest, InsertsOneKey) {
    EXPECT_TRUE(this->tree.insert(42));
    EXPECT_FALSE(this->tree.empty());
    EXPECT_EQ(this->tree.size(), 1U);
    EXPECT_TRUE(this->tree.contains(42));
}

TYPED_TEST(BasicBTreeTest, FindsKeysInASmallTree) {
    this->tree.insert(20);
    this->tree.insert(10);
    this->tree.insert(30);

    EXPECT_EQ(this->tree.size(), 3U);
    EXPECT_TRUE(this->tree.contains(10));
    EXPECT_TRUE(this->tree.contains(20));
    EXPECT_TRUE(this->tree.contains(30));
    EXPECT_FALSE(this->tree.contains(25));
}

TYPED_TEST(BasicBTreeTest, RejectsDuplicatesWithoutChangingContents) {
    std::set<int> expected;
    const std::vector<int> keys{20, 10, 30, 20, 10};

    InsertAll(this->tree, expected, keys);
    EXPECT_EQ(this->tree.size(), 3U);
    ExpectMatches(this->tree, expected);
}

TYPED_TEST(BasicBTreeTest, HandlesAscendingInsertionsAcrossMultipleLevels) {
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);

    InsertAll(this->tree, expected, keys);
    ExpectMatches(this->tree, expected);
}

TYPED_TEST(BasicBTreeTest, HandlesDescendingInsertionsAcrossMultipleLevels) {
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    std::reverse(keys.begin(), keys.end());

    InsertAll(this->tree, expected, keys);
    ExpectMatches(this->tree, expected);
}

TYPED_TEST(BasicBTreeTest, MatchesSetForDeterministicMixedInsertions) {
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20261123);
    std::shuffle(keys.begin(), keys.end(), random);
    keys.insert(keys.end(), {17, 42, 17, 199, 0});

    InsertAll(this->tree, expected, keys);
    ExpectMatches(this->tree, expected);
}

TYPED_TEST(BasicBTreeTest, ErasesKeysAndHandlesMissingKeys) {
    std::set<int> expected;
    InsertAll(this->tree, expected, {10, 20, 30, 40, 50});

    EXPECT_FALSE(this->tree.erase(25));
    ExpectMatches(this->tree, expected);

    for (int key : {30, 10, 50, 20, 40}) {
        ASSERT_TRUE(this->tree.erase(key)) << "key=" << key;
        expected.erase(key);
        ExpectMatches(this->tree, expected);
        EXPECT_FALSE(this->tree.erase(key));
    }

    EXPECT_TRUE(this->tree.insert(7));
    EXPECT_TRUE(this->tree.contains(7));
}

TYPED_TEST(BasicBTreeTest, MatchesSetForMixedDeletionsAcrossMultipleLevels) {
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20261125);
    std::shuffle(keys.begin(), keys.end(), random);
    InsertAll(this->tree, expected, keys);

    std::shuffle(keys.begin(), keys.end(), random);
    for (int key : keys) {
        ASSERT_TRUE(this->tree.erase(key)) << "key=" << key;
        expected.erase(key);
        ExpectMatches(this->tree, expected);
    }
}

TYPED_TEST(BasicBTreeTest, MatchesSetForInterleavedInsertionsAndDeletions) {
    std::set<int> expected;
    std::mt19937 random(20261126);
    std::uniform_int_distribution<int> keys(0, 199);
    std::bernoulli_distribution insert(0.5);

    for (int step = 0; step < 1500; ++step) {
        const int key = keys(random);
        if (insert(random)) {
            EXPECT_EQ(this->tree.insert(key), expected.insert(key).second)
                << "step=" << step << " key=" << key;
        } else {
            EXPECT_EQ(this->tree.erase(key), expected.erase(key) != 0)
                << "step=" << step << " key=" << key;
        }

        EXPECT_EQ(this->tree.size(), expected.size()) << "step=" << step;
        EXPECT_EQ(this->tree.contains(key), expected.contains(key)) << "step=" << step;
        if (step % 25 == 0) {
            ExpectMatches(this->tree, expected);
        }
    }

    ExpectMatches(this->tree, expected);
}

TYPED_TEST(BasicBTreeTest, ErasesInAscendingAndDescendingOrder) {
    for (bool ascending : {true, false}) {
        TypeParam tree;
        std::set<int> expected;
        std::vector<int> keys(200);
        std::iota(keys.begin(), keys.end(), 0);
        InsertAll(tree, expected, keys);

        if (!ascending) {
            std::reverse(keys.begin(), keys.end());
        }

        for (int key : keys) {
            ASSERT_TRUE(tree.erase(key)) << "ascending=" << ascending << " key=" << key;
            expected.erase(key);
            if (key % 17 == 0) {
                ExpectMatches(tree, expected);
            }
        }

        ExpectMatches(tree, expected);
    }
}

TEST(BasicBTreeEraseTest, ErasesInternalKeyWithSpareLeftChild) {
    iheay::BasicBTree<int, 2> tree;
    std::set<int> expected;
    InsertAll(tree, expected, {1, 2, 3, 0});

    ASSERT_TRUE(tree.erase(2));
    expected.erase(2);
    ExpectMatches(tree, expected);
}

TEST(BasicBTreeEraseTest, ErasesInternalKeyWithSpareRightChild) {
    iheay::BasicBTree<int, 2> tree;
    std::set<int> expected;
    InsertAll(tree, expected, {1, 2, 3, 4});

    ASSERT_TRUE(tree.erase(2));
    expected.erase(2);
    ExpectMatches(tree, expected);
}

TEST(BasicBTreeEraseTest, ErasesInternalKeyWhenBothChildrenAreLow) {
    iheay::BasicBTree<int, 2> tree;
    std::set<int> expected;
    InsertAll(tree, expected, {1, 2, 3, 4});

    ASSERT_TRUE(tree.erase(4));
    expected.erase(4);
    ASSERT_TRUE(tree.erase(2));
    expected.erase(2);
    ExpectMatches(tree, expected);
}

TEST(BasicBTreeEraseTest, ContractsRootAfterMergeWhileSearchingForMissingKey) {
    iheay::BasicBTree<int, 2> tree;
    for (int key : {1, 2, 3, 4}) {
        ASSERT_TRUE(tree.insert(key));
    }

    ASSERT_TRUE(tree.erase(4));
    EXPECT_FALSE(tree.erase(0));
    EXPECT_EQ(tree.size(), 3U);

    for (int key : {1, 2, 3}) {
        EXPECT_TRUE(tree.contains(key));
        EXPECT_TRUE(tree.erase(key));
    }
    EXPECT_TRUE(tree.empty());
}

}  // namespace
