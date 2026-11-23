add_executable(b_tree_tests basic_btree_tests.cpp)

target_compile_features(b_tree_tests PRIVATE cxx_std_20)
target_include_directories(b_tree_tests PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(b_tree_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(b_tree_tests)
