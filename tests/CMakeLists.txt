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

add_subdirectory(btree)
add_subdirectory(search)
