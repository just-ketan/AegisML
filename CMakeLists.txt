cmake_minimum_required(VERSION 3.20)

project(
    AegisML
    VERSION 0.1.0
    DESCRIPTION "Low-latency market intelligence and ML platform"
    LANGUAGES CXX
)

# ------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

FetchContent_MakeAvailable(googletest)

# ------------------------------------------------------------
# C++ Configuration
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Compiler Warnings
# ------------------------------------------------------------

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# ------------------------------------------------------------
# Market Simulator Library
# ------------------------------------------------------------

add_library(
    market_simulator
    STATIC

    # Common
    cpp/market_simulator/core/common/symbol.cpp

    # Events
    cpp/market_simulator/core/events/market_event.cpp

    # Orders
    cpp/market_simulator/core/orders/order_manager.cpp

    # Simulator
    cpp/market_simulator/core/simulator/market_simulator.cpp
)

target_include_directories(
    market_simulator
    PUBLIC
        cpp/market_simulator/include
)

# ------------------------------------------------------------
# Simulator Executable
# ------------------------------------------------------------

add_executable(
    aegisml_sim
    cpp/market_simulator/core/tools/main.cpp
)

target_link_libraries(
    aegisml_sim
    PRIVATE
        market_simulator
)

# ------------------------------------------------------------
# Testing
# ------------------------------------------------------------

include(GoogleTest)

# Domain Tests
add_executable(
    market_event_tests
    tests/domain/market_event_test.cpp
)

target_link_libraries(
    market_event_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(market_event_tests)


add_executable(
    order_tests
    tests/domain/order_test.cpp
)

target_link_libraries(
    order_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(order_tests)


# Order Tests
add_executable(
    order_manager_tests
    tests/orders/order_manager_test.cpp
)

target_link_libraries(
    order_manager_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(order_manager_tests)


# Simulator Tests
add_executable(
    market_simulator_tests
    tests/simulator/market_simulator_test.cpp
)

target_link_libraries(
    market_simulator_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(market_simulator_tests)

# ------------------------------------------------------------
# Development Tools
# ------------------------------------------------------------

add_executable(
    memory_probe
    cpp/market_simulator/core/tools/memory_probe.cpp
)

target_link_libraries(
    memory_probe
    PRIVATE
        market_simulator
)