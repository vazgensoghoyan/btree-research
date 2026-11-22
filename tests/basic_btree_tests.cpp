#include "btree/impl/basic_btree.h"

#include <gtest/gtest.h>

TEST(BasicBTreeTest, StartsEmpty) {
    iheay::BasicBTree<int> tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0U);
}
