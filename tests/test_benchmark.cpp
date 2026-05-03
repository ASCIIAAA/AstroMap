//  AstroMap – Benchmark Test Driver
//
//  Build (from project root, Release for fair numbers):
//    cmake -B build -DCMAKE_BUILD_TYPE=Release
//    cmake --build build --target test_benchmark
//
//  Run:
//    ./build/test_benchmark --bench          quick  (N = 1k, 10k)
//    ./build/test_benchmark --bench-full     full   (N = 1k, 10k, 100k)
//    ./build/test_benchmark --bench-csv      full + export benchmark_results.csv
//    ./build/test_benchmark --help

#include "../include/Benchmark.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

static bool has(const std::vector<std::string>& args, const std::string& flag) {
    return std::find(args.begin(), args.end(), flag) != args.end();
}

static void usage() {
    std::cout
        << "\nUsage: test_benchmark [OPTION]\n\n"
        << "  --bench          Quick run  (N = 1 000, 10 000)\n"
        << "  --bench-full     Full  run  (N = 1 000, 10 000, 100 000)\n"
        << "  --bench-csv      Full  run  + export to benchmark_results.csv\n"
        << "  --help           Show this message\n\n"
        << "Structures benchmarked:\n"
        << "  Octree        – insert, queryRange\n"
        << "  VEB Tree      – insert, contains, successor\n"
        << "  Suffix Tree   – addPattern, search, hasPattern\n"
        << "  Fibonacci Heap– insert, extractMin\n"
        << "  Range Tree    – insert, query2D, queryMag, queryTemp\n\n";
}

int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty() || has(args, "--help")) {
        usage();
        return 0;
    }

    AstroMapBenchmark bench;

    if (has(args, "--bench")) {
        std::cout << ">>> Quick benchmark (N = 1 000 and 10 000)\n";
        bench.runAll({1000, 10000});
        bench.printReport();
        return 0;
    }

    if (has(args, "--bench-full") || has(args, "--bench-csv")) {
        std::cout << ">>> Full benchmark (N = 1 000 / 10 000 / 100 000)\n";
        bench.runAll({1000, 10000, 100000});
        bench.printReport();
        if (has(args, "--bench-csv"))
            bench.exportCSV("benchmark_results.csv");
        return 0;
    }

    usage();
    return 0;
}