# Tactical Radio Simulator

A C++20 tactical radio simulator.

## Requirements (macOS)

- Xcode Command Line Tools (provides Apple Clang): `xcode-select --install`
- CMake 3.24 or newer: `brew install cmake`

GoogleTest is downloaded automatically by CMake the first time you configure,
so an internet connection is needed on that first run.

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Run the simulator

```sh
./build/radio_sim
```

## Run the tests

```sh
ctest --test-dir build --output-on-failure
```

You can also run the test binary directly for GoogleTest's more detailed output:

```sh
./build/radio_sim_tests
```

## Check for data races (ThreadSanitizer)

`Radio` can be shared between threads. To check that no data races sneak
in, build a separate copy with ThreadSanitizer enabled and run the tests:

```sh
cmake -S . -B build-tsan -DRADIO_SIM_ENABLE_TSAN=ON
cmake --build build-tsan
ctest --test-dir build-tsan --output-on-failure
```

Any race is printed as `WARNING: ThreadSanitizer: data race`, with the file
and line of both conflicting accesses, and the affected test is marked as
failed. Run the tests through `ctest` as shown: running the test binary
directly still prints `PASSED` when a race is found, so warnings are easy to
miss.

## Project layout

```
include/   Public headers
src/       Application source code
tests/     GoogleTest unit tests
```
