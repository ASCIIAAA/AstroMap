cmake_minimum_required(VERSION 3.16)
project(AstroMap VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Compiler flags ───────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# ── Include paths ─────────────────────────────────────────────────
include_directories(include)

# ── Engine source files (shared library target) ───────────────────
set(ENGINE_SOURCES
    src/Octree.cpp
    src/vEBTree.cpp
    src/FibonacciHeap.cpp
    src/SuffixTree.cpp
    src/RangeTree.cpp
    src/GalacticEngine.cpp    # matches the filename in the repo
    src/Benchmark.cpp
    src/AsciiStarMap.cpp
    src/advanced_structures.cpp
)

# ── Static library so all executables share the same object files ─
add_library(astromap_lib STATIC ${ENGINE_SOURCES})

target_include_directories(astromap_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(astromap_lib PUBLIC cxx_std_17)

# ── Main executable ───────────────────────────────────────────────
add_executable(astromap main.cpp)
target_link_libraries(astromap PRIVATE astromap_lib)

# ── Unit tests ────────────────────────────────────────────────────
add_executable(test_octree tests/test_octree.cpp)
target_link_libraries(test_octree PRIVATE astromap_lib)

add_executable(test_veb tests/test_veb.cpp)
target_link_libraries(test_veb PRIVATE astromap_lib)

# ── Benchmark test executable ─────────────────────────────────────
add_executable(test_benchmark tests/test_benchmark.cpp)
target_link_libraries(test_benchmark PRIVATE astromap_lib)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/web DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Release build gives real numbers; Debug is fine for correctness checks
# cmake -B build -DCMAKE_BUILD_TYPE=Release
# cmake --build build --target test_benchmark
# ./build/test_benchmark --bench

add_executable(test_advanced tests/test_advanced.cpp)
target_link_libraries(test_advanced PRIVATE astromap_lib)