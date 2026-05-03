#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

//  Declares BenchmarkSuite (generic timing harness) and
//  AstroMapBenchmark (structure-specific experiments).

#include <string>
#include <vector>
#include <functional>
#include <chrono>

// Result record (one row in the report)
struct BenchmarkResult {
    std::string structureName;
    std::string operationName;
    size_t      dataSize    = 0;
    double      elapsedMs   = 0.0;
    double      opsPerSecond= 0.0;
    std::string complexity;
    std::string notes;
};

// Generic harness 
class BenchmarkSuite {
public:
    // Register one experiment.
    // fn receives the dataSize so it can loop over it.
    void add(const std::string& name,
             const std::string& operation,
             size_t             dataSize,
             const std::string& complexity,
             std::function<void(size_t)> fn,
             const std::string& notes = "");

    // Run every registered experiment and store results.
    void runAll();

    // Clear registered experiments and results.
    void reset();

    // Human-readable table to stdout.
    void printReport() const;

    // CSV export.
    void exportCSV(const std::string& filename) const;

    const std::vector<BenchmarkResult>& results() const { return results_; }

private:
    struct Entry {
        std::string name;
        std::string operation;
        std::string complexity;
        std::string notes;
        size_t      dataSize;
        std::function<void(size_t)> fn;
    };

    std::vector<Entry>           entries_;
    std::vector<BenchmarkResult> results_;

    // Returns wall-clock milliseconds for one call to fn().
    double timeMs(std::function<void()> fn) const;
};

// AstroMap-specific benchmark driver
class AstroMapBenchmark {
public:
    // Run all structure benchmarks for every N in scaleSizes.
    // Typical call: bench.runAll({1000, 10000, 100000});
    void runAll(const std::vector<size_t>& scaleSizes);

    void printReport() const;
    void exportCSV(const std::string& filename) const;

private:
    BenchmarkSuite suite_;

    // One method per data structure
    void benchmarkOctree    (size_t n);
    void benchmarkVEBTree   (size_t n);
    void benchmarkSuffixTree(size_t n);
    void benchmarkFibHeap   (size_t n);
    void benchmarkRangeTree (size_t n);
};

#endif