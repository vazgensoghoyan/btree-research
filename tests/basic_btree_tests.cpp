#include "btree/impl/basic_btree.h"

#include <gtest/gtest.h>

TEST(BasicBTreeTest, StartsEmpty) {
    iheay::BasicBTree<int> tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0U);
}

TEST(BasicBTreeTest, InsertsOneKey) {
    iheay::BasicBTree<int> tree;

    EXPECT_TRUE(tree.insert(42));
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1U);
    EXPECT_TRUE(tree.contains(42));
}

TEST(BasicBTreeTest, FindsKeysInASmallTree) {
    iheay::BasicBTree<int> tree;
    tree.insert(20);
    tree.insert(10);
    tree.insert(30);

    EXPECT_EQ(tree.size(), 3U);
    EXPECT_TRUE(tree.contains(10));
    EXPECT_TRUE(tree.contains(20));
    EXPECT_TRUE(tree.contains(30));
    EXPECT_FALSE(tree.contains(25));
}
