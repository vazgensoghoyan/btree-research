#include "btree/impl/basic_btree.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <vector>

namespace {

using TestTree = iheay::BasicBTree<int, 2>;

void expect_matches(const TestTree& tree, const std::set<int>& expected) {
    ASSERT_EQ(tree.size(), expected.size());
    ASSERT_EQ(tree.empty(), expected.empty());

    for (int key = -5; key <= 205; ++key) {
        ASSERT_EQ(tree.contains(key), expected.contains(key)) << "key=" << key;
    }
}

void insert_all(TestTree& tree, std::set<int>& expected,
                const std::vector<int>& keys) {
    for (int key : keys) {
        const bool inserted = expected.insert(key).second;
        ASSERT_EQ(tree.insert(key), inserted) << "key=" << key;
    }
}

}  // namespace

TEST(BasicBTreeTest, StartsEmpty) {
    TestTree tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0U);
}

TEST(BasicBTreeTest, InsertsOneKey) {
    TestTree tree;

    EXPECT_TRUE(tree.insert(42));
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1U);
    EXPECT_TRUE(tree.contains(42));
}

TEST(BasicBTreeTest, FindsKeysInASmallTree) {
    TestTree tree;
    tree.insert(20);
    tree.insert(10);
    tree.insert(30);

    EXPECT_EQ(tree.size(), 3U);
    EXPECT_TRUE(tree.contains(10));
    EXPECT_TRUE(tree.contains(20));
    EXPECT_TRUE(tree.contains(30));
    EXPECT_FALSE(tree.contains(25));
}

TEST(BasicBTreeTest, RejectsDuplicateKeysWithoutChangingContents) {
    TestTree tree;
    std::set<int> expected;
    const std::vector<int> keys = {20, 10, 30, 20, 10};
    insert_all(tree, expected, keys);

    EXPECT_EQ(tree.size(), 3U);
    expect_matches(tree, expected);
}

TEST(BasicBTreeTest, HandlesAscendingInsertionsAcrossMultipleLevels) {
    TestTree tree;
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);

    insert_all(tree, expected, keys);
    expect_matches(tree, expected);
}

TEST(BasicBTreeTest, HandlesDescendingInsertionsAcrossMultipleLevels) {
    TestTree tree;
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    std::reverse(keys.begin(), keys.end());

    insert_all(tree, expected, keys);
    expect_matches(tree, expected);
}

TEST(BasicBTreeTest, MatchesSetForDeterministicMixedInsertions) {
    TestTree tree;
    std::set<int> expected;
    std::vector<int> keys(200);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20261123);
    std::shuffle(keys.begin(), keys.end(), random);
    keys.insert(keys.end(), {17, 42, 17, 199, 0});

    insert_all(tree, expected, keys);
    expect_matches(tree, expected);
}
