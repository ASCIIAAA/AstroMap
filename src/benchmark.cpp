#include "../include/Benchmark.hpp"
#include "../include/structures/Octree.hpp"
#include "../include/structures/vEBTree.hpp"
#include "../include/structures/SuffixTree.hpp"
#include "../include/structures/FibonacciHeap.hpp"
#include "../include/structures/RangeTree.hpp"
#include "../include/core/Star.hpp"
#include "../include/core/Vector3D.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <numeric>
#include <chrono>
#include <memory>

//  BenchmarkSuite

void BenchmarkSuite::add(const std::string& name,
                          const std::string& operation,
                          size_t             dataSize,
                          const std::string& complexity,
                          std::function<void(size_t)> fn,
                          const std::string& notes)
{
    entries_.push_back({name, operation, complexity, notes, dataSize, fn});
}

void BenchmarkSuite::runAll()
{
    results_.clear();
    for (auto& e : entries_) {
        // repeat each experiment until at least MIN_MS has elapsed,
        // then divide to get a per-run average. This prevents 0.000 ms
        // results on fast operations without artificially inflating N.
        constexpr double MIN_MS      = 20.0;  // run for at least 20 ms
        constexpr int    MAX_REPEATS = 10000;

        int    reps    = 0;
        double totalMs = 0.0;

        // Warm-up run (not counted) to avoid cold-cache bias.
        e.fn(e.dataSize);

        do {
            totalMs += timeMs([&]() { e.fn(e.dataSize); });
            ++reps;
        } while (totalMs < MIN_MS && reps < MAX_REPEATS);

        double ms = totalMs / static_cast<double>(reps);

        BenchmarkResult r;
        r.structureName = e.name;
        r.operationName = e.operation;
        r.dataSize      = e.dataSize;
        r.elapsedMs     = ms;
        r.opsPerSecond  = (ms > 0.0)
                          ? static_cast<double>(e.dataSize) / (ms / 1000.0)
                          : 0.0;
        r.complexity    = e.complexity;
        r.notes         = e.notes;
        results_.push_back(r);
    }
}

void BenchmarkSuite::reset()
{
    entries_.clear();
    results_.clear();
}

double BenchmarkSuite::timeMs(std::function<void()> fn) const
{
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();
    fn();
    auto t1 = Clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// Report

void BenchmarkSuite::printReport() const
{
    if (results_.empty()) {
        std::cout << "[Benchmark] No results - call runAll() first.\n";
        return;
    }

    constexpr int W_STRUCT = 18;
    constexpr int W_OP     = 14;
    constexpr int W_N      = 10;
    constexpr int W_MS     = 12;
    constexpr int W_OPS    = 16;
    constexpr int W_CMPLX  = 16;
    constexpr int W_NOTES  = 30;
    constexpr int TOTAL    = W_STRUCT+W_OP+W_N+W_MS+W_OPS+W_CMPLX+W_NOTES;

    auto hline = [&](char c = '-') {
        std::cout << std::string(TOTAL, c) << "\n";
    };

    hline('=');
    std::cout << "  ASTRO-MAP  BENCHMARK  REPORT\n";
    hline('=');
    std::cout << std::left
              << std::setw(W_STRUCT) << "Structure"
              << std::setw(W_OP)     << "Operation"
              << std::setw(W_N)      << "N"
              << std::setw(W_MS)     << "Time (ms)"
              << std::setw(W_OPS)    << "Ops / sec"
              << std::setw(W_CMPLX)  << "Complexity"
              << std::setw(W_NOTES)  << "Notes"
              << "\n";
    hline();

    std::string lastStruct;
    for (const auto& r : results_) {
        if (!lastStruct.empty() && r.structureName != lastStruct)
            hline();
        lastStruct = r.structureName;

        std::ostringstream ops;
        if      (r.opsPerSecond >= 1e6) ops << std::fixed << std::setprecision(2) << r.opsPerSecond/1e6 << " M";
        else if (r.opsPerSecond >= 1e3) ops << std::fixed << std::setprecision(2) << r.opsPerSecond/1e3 << " K";
        else                            ops << std::fixed << std::setprecision(0) << r.opsPerSecond;

        std::cout << std::left
                  << std::setw(W_STRUCT) << r.structureName
                  << std::setw(W_OP)     << r.operationName
                  << std::setw(W_N)      << r.dataSize
                  << std::setw(W_MS)     << std::fixed << std::setprecision(3) << r.elapsedMs
                  << std::setw(W_OPS)    << ops.str()
                  << std::setw(W_CMPLX)  << r.complexity
                  << std::setw(W_NOTES)  << r.notes
                  << "\n";
    }
    hline('=');
}

void BenchmarkSuite::exportCSV(const std::string& filename) const
{
    std::ofstream f(filename);
    if (!f) { std::cerr << "[Benchmark] Cannot open " << filename << "\n"; return; }
    f << "Structure,Operation,N,Time_ms,Ops_per_sec,Complexity,Notes\n";
    for (const auto& r : results_)
        f << r.structureName << "," << r.operationName << ","
          << r.dataSize << ","
          << std::fixed << std::setprecision(4) << r.elapsedMs << ","
          << std::fixed << std::setprecision(2) << r.opsPerSecond << ","
          << r.complexity << "," << r.notes << "\n";
    std::cout << "[Benchmark] Exported to " << filename << "\n";
}

//  Shared RNG helpers
static std::mt19937_64 rng(42);

static double randDouble(double lo, double hi) {
    return std::uniform_real_distribution<double>(lo, hi)(rng);
}
static float randFloat(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng);
}

using StarVec = std::shared_ptr<std::vector<Star*>>;

static StarVec makeStars(size_t n)
{
    auto out = std::make_shared<std::vector<Star*>>();
    out->reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Star* s      = new Star();
        s->id        = static_cast<uint64_t>(i + 1);
        s->name      = "S" + std::to_string(i + 1);
        s->position  = Vector3D(randDouble(-1000, 1000),
                                randDouble(-1000, 1000),
                                randDouble(-1000, 1000));
        s->magnitude    = randDouble(1.0, 15.0);
        s->temperature  = randFloat(3000.f, 30000.f);
        out->push_back(s);
    }
    return out;
}

static std::string makeLightCurve(size_t len) {
    std::uniform_int_distribution<int> pick(0, 4);
    std::string s(len, ' ');
    for (auto& c : s) c = static_cast<char>('A' + pick(rng));
    return s;
}


//  AstroMapBenchmark  - individual structure benchmarks
void AstroMapBenchmark::benchmarkOctree(size_t n)
{
    StarVec stars = makeStars(n);

    auto makeBound = [](Vector3D c, double half) -> Boundary {
        return Boundary{c, half};
    };

    // INSERT — rebuilds tree each repeat; fast enough to need repeating.
    suite_.add("Octree", "Insert", n, "O(log n)",
        [stars, makeBound](size_t sz) {
            Octree tree(makeBound(Vector3D(0,0,0), 1100.0));
            for (size_t i = 0; i < sz; ++i)
                tree.insert((*stars)[i]);
        },
        "3-D spatial insert");

    // RANGE QUERY — pre-built tree, 20 queries per call.
    auto built = std::make_shared<Octree>(makeBound(Vector3D(0,0,0), 1100.0));
    for (auto* s : *stars) built->insert(s);

    suite_.add("Octree", "QueryRange", n, "O(k + log n)",
        [built, makeBound](size_t) {
            for (int i = 0; i < 20; ++i) {
                Boundary q = makeBound(
                    Vector3D(randDouble(-500,500),
                             randDouble(-500,500),
                             randDouble(-500,500)),
                    200.0);
                std::vector<Star*> found;
                built->queryRange(q, found);
            }
        },
        "20 x radius-200 queries");
}

void AstroMapBenchmark::benchmarkVEBTree(size_t n)
{
    uint64_t universe = 1;
    while (universe < n * 2) universe <<= 1;

    auto ids = std::make_shared<std::vector<uint64_t>>(n);
    std::iota(ids->begin(), ids->end(), uint64_t(1));
    std::shuffle(ids->begin(), ids->end(), rng);

    suite_.add("VEB Tree", "Insert", n, "O(log log U)",
        [universe, ids](size_t sz) {
            vEBTree veb(universe);
            for (size_t i = 0; i < sz; ++i)
                veb.insert((*ids)[i]);
        },
        "universe=" + std::to_string(universe));

    auto veb = std::make_shared<vEBTree>(universe);
    for (auto id : *ids) veb->insert(id);

    suite_.add("VEB Tree", "Contains", n, "O(log log U)",
        [veb, ids](size_t sz) {
            for (size_t i = 0; i < sz; ++i)
                veb->contains((*ids)[i]);
        },
        "100% hit rate");

    suite_.add("VEB Tree", "Successor", n, "O(log log U)",
        [veb, ids](size_t sz) {
            for (size_t i = 0; i < sz; ++i)
                veb->successor((*ids)[i]);
        },
        "next star ID");
}

void AstroMapBenchmark::benchmarkSuffixTree(size_t n)
{
    size_t numStars  = std::min(n, size_t(5000));
    size_t curveLen  = 20;

    auto curves = std::make_shared<std::vector<std::string>>(numStars);
    for (auto& c : *curves) c = makeLightCurve(curveLen);

    suite_.add("Suffix Tree", "AddPattern", numStars, "O(n) Ukkonen",
        [curves, numStars](size_t) {
            SuffixTree st;
            for (size_t i = 0; i < numStars; ++i)
                st.addPattern(static_cast<uint64_t>(i + 1), (*curves)[i]);
        },
        std::to_string(numStars) + " stars, len=" + std::to_string(curveLen));

    auto st = std::make_shared<SuffixTree>();
    for (size_t i = 0; i < numStars; ++i)
        st->addPattern(static_cast<uint64_t>(i + 1), (*curves)[i]);

    suite_.add("Suffix Tree", "Search", 100, "O(m)",
        [st](size_t) {
            for (int i = 0; i < 100; ++i) {
                std::string pat = makeLightCurve(4);
                st->search(pat);
            }
        },
        "100 x pattern-len-4");

    suite_.add("Suffix Tree", "HasPattern", 100, "O(m)",
        [st](size_t) {
            for (int i = 0; i < 100; ++i) {
                std::string pat = makeLightCurve(4);
                st->hasPattern(pat);
            }
        },
        "bool existence check");
}

void AstroMapBenchmark::benchmarkFibHeap(size_t n)
{
    auto makeEvent = [](size_t i) -> AstroEvent {
        return AstroEvent{
            randDouble(0.0, 1e6),
            static_cast<uint64_t>(i + 1),
            "event_" + std::to_string(i)
        };
    };

    suite_.add("Fibonacci Heap", "Insert", n, "O(1) amort.",
        [makeEvent](size_t sz) {
            FibonacciHeap heap;
            for (size_t i = 0; i < sz; ++i)
                heap.insert(makeEvent(i));
        },
        "AstroEvent priority insert");

    // capture n and makeEvent; rebuild heap each repeat
    size_t extractions = std::min(n, size_t(1000));
    suite_.add("Fibonacci Heap", "ExtractMin", extractions, "O(log n) amort.",
        [makeEvent, n, extractions](size_t sz) {
            FibonacciHeap heap;                          // fresh heap each time
            for (size_t i = 0; i < n; ++i)
                heap.insert(makeEvent(i));
            for (size_t i = 0; i < extractions && !heap.isEmpty(); ++i)
                heap.extractMin();
        },
        std::to_string(extractions) + " extract-mins");
}

void AstroMapBenchmark::benchmarkRangeTree(size_t n)
{
    StarVec stars = makeStars(n);

    suite_.add("Range Tree", "Insert", n, "O(log^2 n)",
        [stars](size_t sz) {
            RangeTree rt;
            for (size_t i = 0; i < sz; ++i)
                rt.insert((*stars)[i]);
        },
        "mag+temp indexed insert");

    auto rt = std::make_shared<RangeTree>();
    for (auto* s : *stars) rt->insert(s);

    suite_.add("Range Tree", "Query2D", n, "O(log^2 n + k)",
        [rt](size_t) {
            for (int i = 0; i < 20; ++i) {
                float magLo  = randFloat(1.f,  10.f);
                float magHi  = magLo + randFloat(1.f, 4.f);
                float tempLo = randFloat(3000.f, 20000.f);
                float tempHi = tempLo + randFloat(1000.f, 5000.f);
                rt->query(magLo, magHi, tempLo, tempHi);
            }
        },
        "20 x mag+temp queries");

    suite_.add("Range Tree", "QueryMag", n, "O(log n + k)",
        [rt](size_t) {
            for (int i = 0; i < 20; ++i) {
                float lo = randFloat(1.f, 10.f);
                rt->queryByMagnitude(lo, lo + 3.f);
            }
        },
        "20 x magnitude-only");

    suite_.add("Range Tree", "QueryTemp", n, "O(log n + k)",
        [rt](size_t) {
            for (int i = 0; i < 20; ++i) {
                float lo = randFloat(3000.f, 20000.f);
                rt->queryByTemperature(lo, lo + 4000.f);
            }
        },
        "20 x temperature-only");
}

void AstroMapBenchmark::runAll(const std::vector<size_t>& scaleSizes)
{
    suite_.reset();
    std::cout << "\n[AstroMap Benchmark] Registering experiments...\n";

    for (size_t n : scaleSizes) {
        std::cout << "  N = " << n << "\n";
        benchmarkOctree    (n);
        benchmarkVEBTree   (n);
        benchmarkSuffixTree(n);
        benchmarkFibHeap   (n);
        benchmarkRangeTree (n);
    }

    std::cout << "\n[AstroMap Benchmark] Running...\n\n";
    suite_.runAll();
}

void AstroMapBenchmark::printReport() const { suite_.printReport(); }
void AstroMapBenchmark::exportCSV(const std::string& f) const { suite_.exportCSV(f); }