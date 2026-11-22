# B-tree practice

Configure, build, and run the tests:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The test target uses an installed GoogleTest package when available. Otherwise,
CMake fetches GoogleTest v1.17.0. Pass `-DBUILD_TESTING=OFF` during configuration
to build the playground without tests.
