cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

FetchContent_MakeAvailable(googletest)

project(
    AegisML
    VERSION 0.1.0
    DESCRIPTION "Low-latency market intelligence and ML platform"
    LANGUAGES CXX
)

# C++ configuration

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# Market Simulator Library

add_library(
    market_simulator
    STATIC
    cpp/market_simulator/src/market_event.cpp
    cpp/market_simulator/src/market_simulator.cpp
    cpp/market_simulator/src/symbol.cpp
    cpp/market_simulator/src/order_manager.cpp
)

target_include_directories(
    market_simulator
    PUBLIC
        cpp/market_simulator/include
)

# Simulator executable

add_executable(
    aegisml_sim
    cpp/market_simulator/src/main.cpp
)

target_link_libraries(
    aegisml_sim
    PRIVATE
        market_simulator
)


# Testing
include(GoogleTest)

add_executable(
    market_simulator_tests
    tests/market_simulator_test.cpp
)

target_link_libraries(
    market_simulator_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(market_simulator_tests)

add_executable(
    market_event_tests
    tests/market_event_test.cpp
)

target_link_libraries(
    market_event_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(market_event_tests)


#----------------------------
add_executable(
    memory_probe
    cpp/market_simulator/src/memory_probe.cpp
)

target_link_libraries(
    memory_probe
    PRIVATE
        market_simulator
)

add_executable(
    order_tests
    tests/order_test.cpp
)

target_link_libraries(
    order_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(order_tests)

add_executable(
    order_manager_tests
    tests/order_manager_test.cpp
)

target_link_libraries(
    order_manager_tests
    PRIVATE
        market_simulator
        GTest::gtest_main
)

gtest_discover_tests(order_manager_tests)