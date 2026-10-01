cmake_minimum_required(VERSION 3.24)

project(TacticalRadioSim
    VERSION 0.1.0
    DESCRIPTION "Tactical radio simulator"
    LANGUAGES CXX
)

# Require C++20 and disable compiler-specific extensions (e.g. gnu++20).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit compile_commands.json for editors and clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ThreadSanitizer detects data races at runtime. It must apply to every
# target (including GoogleTest), so it's set globally before any targets.
option(RADIO_SIM_ENABLE_TSAN "Build with ThreadSanitizer" OFF)
if(RADIO_SIM_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()

find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Core library: all simulator logic lives here so both the app and the tests
# can link against it.
# ---------------------------------------------------------------------------
add_library(radio_core
    src/radio.cpp
    src/message_queue.cpp
)
target_include_directories(radio_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(radio_core PUBLIC Threads::Threads)
target_compile_options(radio_core PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(radio_sim src/main.cpp)
target_link_libraries(radio_sim PRIVATE radio_core)
target_compile_options(radio_sim PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
option(RADIO_SIM_BUILD_TESTS "Build the unit tests" ON)

if(RADIO_SIM_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # Don't install GoogleTest alongside this project.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(radio_sim_tests
        tests/radio_test.cpp
        tests/radio_thread_test.cpp
        tests/message_queue_test.cpp
    )
    target_link_libraries(radio_sim_tests PRIVATE radio_core GTest::gtest_main)

    include(GoogleTest)
    # The timeout turns a deadlocked (hanging) test into a failure.
    gtest_discover_tests(radio_sim_tests PROPERTIES TIMEOUT 10)
endif()
