cmake_minimum_required(VERSION 3.20)

project(b_tree LANGUAGES CXX)

add_executable(b_tree
    src/main.cpp
)

target_compile_features(b_tree PRIVATE cxx_std_20)

target_include_directories(b_tree
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(b_tree PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

include(CTest)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
