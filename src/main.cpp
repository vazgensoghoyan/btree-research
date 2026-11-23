#include <iostream>
#include <vector>
#include <cstddef>

#include "btree/search/binary_search.h"

int main() {
    std::vector<int32_t> nums = { 1, 3, 4, 4, 4, 6, 7, 9, 9 };

    iheay::search::BinarySearch engine{};

    std::span<const int32_t> nums_view{nums};
    int32_t key = 4;

    std::size_t index = engine(nums_view, key);

    std::cout << index << std::endl;

    return 0;
}
