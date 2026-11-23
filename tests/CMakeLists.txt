# Test dependency.
find_package(GTest CONFIG QUIET)

if (NOT GTest_FOUND)
	include(FetchContent)

	set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG v1.17.0
	)
	FetchContent_MakeAvailable(googletest)
endif()

# Test executable.
add_executable(b_tree_tests basic_btree_tests.cpp)

target_compile_features(b_tree_tests PRIVATE cxx_std_20)
target_include_directories(b_tree_tests
	PRIVATE
		${PROJECT_SOURCE_DIR}
)
target_link_libraries(b_tree_tests PRIVATE GTest::gtest_main)

# Register each GoogleTest case with CTest.
include(GoogleTest)
gtest_discover_tests(b_tree_tests)
