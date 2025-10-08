# Benchmark harness (P0 baseline) for YAEP
# Provides a simple executable that parses a small built‑in grammar
# and a synthetic token stream to capture timing and memory stats.
# Future phases will extend this to multiple grammars & corpus inputs.

cmake_minimum_required(VERSION 3.15)

project(yaep_bench LANGUAGES C)

add_executable(yaep_bench bench_driver.c)

# Reuse include dirs so we can include yaep.h directly and reach generated sgramm.c includes
target_include_directories(yaep_bench PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

# Build as part of normal build if benchmarks enabled (default ON for baseline capture)
option(YAEP_BUILD_BENCH "Build benchmark harness" ON)
if(YAEP_BUILD_BENCH)
    add_dependencies(yaep_bench yaep_static)
    target_link_libraries(yaep_bench PRIVATE yaep_static)
endif()

# Provide a custom target to run the baseline benchmark and emit JSON.
add_custom_target(bench-baseline
    COMMAND yaep_bench --baseline --iterations 5 --json ${PROJECT_BINARY_DIR}/perf_baseline.json
    DEPENDS yaep_bench
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    USES_TERMINAL
)
