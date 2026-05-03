cmake_minimum_required(VERSION 3.20)
project(ComplexityProject2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(levin_sat
        src/main.cpp
        src/DPLL.cpp
        src/Levin.cpp
        src/Useful.cpp
)

target_compile_options(levin_sat PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        FIND_PACKAGE_ARGS NAMES GTest
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(sat_core STATIC
        src/DPLL.cpp
        src/Levin.cpp
        src/Useful.cpp
)
target_include_directories(sat_core PUBLIC include)

add_executable(run_tests
        tests/tests_dpll.cpp
        tests/tests_levin.cpp
        tests/tests_types.cpp
        tests/tests_useful.cpp
        tests/tests_stress.cpp
)

target_include_directories(run_tests PRIVATE include)
target_link_libraries(run_tests PRIVATE sat_core GTest::gtest_main)
target_compile_definitions(run_tests PRIVATE)

target_compile_options(run_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

include(GoogleTest)
gtest_discover_tests(run_tests)