cmake_minimum_required(VERSION 3.20)
project(cpp_quant_finance_library VERSION 2.0.0 LANGUAGES CXX)

option(QF_BUILD_TESTS "Build the test suite" ON)
option(QF_BUILD_EXAMPLES "Build example applications" ON)

add_library(quantfinance
    src/black_scholes.cpp
    src/binomial_tree.cpp
    src/trinomial_tree.cpp
    src/monte_carlo.cpp
    src/heston.cpp
    src/implied_volatility.cpp
    src/fixed_income.cpp
    src/risk.cpp
)
add_library(QuantFinance::quantfinance ALIAS quantfinance)

target_include_directories(quantfinance
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(quantfinance PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(quantfinance PRIVATE /W4 /permissive-)
else()
    target_compile_options(quantfinance PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(QF_BUILD_EXAMPLES)
    add_executable(pricing_demo examples/pricing_demo.cpp)
    target_link_libraries(pricing_demo PRIVATE QuantFinance::quantfinance)
endif()

include(CTest)
if(BUILD_TESTING AND QF_BUILD_TESTS)
    add_executable(quantfinance_tests tests/test_quant_finance.cpp)
    target_link_libraries(quantfinance_tests PRIVATE QuantFinance::quantfinance)
    add_test(NAME quantfinance_tests COMMAND quantfinance_tests)
endif()
