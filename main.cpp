#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include "engine/GalacticEngine.hpp"
#include "structures/advanced_structures.hpp"
#include "core/AsciiStarMap.hpp"

void printStarList(const std::vector<Star*>& stars, const std::string& label) {
    std::cout << "\n[" << label << "] Found " << stars.size() << " star(s):" << std::endl;
    for (Star* s : stars) {
        std::cout << std::fixed << std::setprecision(2)
                  << "  -> [ID:" << s->id << "] " << std::left << std::setw(20) << s->name
                  << "  mag=" << std::setw(6) << s->magnitude
                  << "  temp=" << std::setw(7) << s->temperature
                  << "  pos=" << s->position << std::endl;
    }
}

void exportStarsToJson(const GalacticEngine& engine, const std::string& filename) {
    std::ofstream outFile(filename);
    const auto& stars = engine.getStars();

    outFile << "[\n";
    for (size_t i = 0; i < stars.size(); ++i) {
        outFile << "  {\n";
        outFile << "    \"id\": " << stars[i].id << ",\n";
        outFile << "    \"name\": \"" << stars[i].name << "\",\n";
        outFile << "    \"mag\": " << stars[i].magnitude << ",\n";
        outFile << "    \"temp\": " << stars[i].temperature << ",\n";
        outFile << "    \"pos\": [" << stars[i].position.x << ", "
                << stars[i].position.y << ", " << stars[i].position.z << "]\n";
        outFile << "  }" << (i == stars.size() - 1 ? "" : ",") << "\n";
    }
    outFile << "]";
    outFile.close();
    std::cout << "[Web] Data exported to " << filename << std::endl;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "============================================" << std::endl;
    std::cout << "   Astro-Map Galactic Navigation Engine     " << std::endl;
    std::cout << "============================================\n" << std::endl;

    GalacticEngine engine;
    
    if (!engine.loadStars("data/stars.csv")) return 1;

    exportStarsToJson(engine, "web/stars.json");

    // Light curves and events — loaded once after the single loadStars call.
    engine.loadEvents("data/lightcurves.txt");
    engine.printSummary();

    // ── 1. Octree: Spatial Range Query ────────────────────────────────────
    std::cout << "--- MODULE 1: Octree Spatial Query ---" << std::endl;
    auto nearby = engine.findStarsInRegion(Vector3D(10.0, 0.0, 0.0), 8.0);
    printStarList(nearby, "Stars within 8ly of (10,0,0)");

    Star* nearest = engine.findNearestStar(Vector3D(0.1, 0.1, 0.1));
    if (nearest) {
        std::cout << "\n  Nearest star to origin: " << nearest->name
                  << " (ID:" << nearest->id << ")\n";
    }

    // ── 2. vEB Tree: ID Lookup ────────────────────────────────────────────
    std::cout << "\n--- MODULE 2: Van Emde Boas ID Lookup ---" << std::endl;
    for (uint64_t testId : {1ULL, 5ULL, 7ULL, 99ULL}) {
        Star* found = engine.lookupById(testId);
        if (found)
            std::cout << "  ID " << testId << " -> " << found->name << std::endl;
        else
            std::cout << "  ID " << testId << " -> NOT FOUND" << std::endl;
    }

    std::cout << "  Star IDs sorted: ";
    for (uint64_t id : {1ULL,2ULL,3ULL,4ULL,5ULL,6ULL,7ULL,8ULL,9ULL,10ULL}) {
        if (engine.starExists(id)) std::cout << id << " ";
    }
    std::cout << std::endl;

    // ── 3. Suffix Tree: Light-Curve Pattern Matching ──────────────────────
    std::cout << "\n--- MODULE 3: Suffix Tree Pattern Matching ---" << std::endl;

    auto matchHL  = engine.findByLightCurvePattern("HL");
    printStarList(matchHL,  "Stars with 'HL' pattern in light-curve");

    auto matchMMM = engine.findByLightCurvePattern("MMM");
    printStarList(matchMMM, "Stars with 'MMM' pattern in light-curve");

    auto matchHHH = engine.findByLightCurvePattern("HHH");
    printStarList(matchHHH, "Stars with 'HHH' pattern");

    // ── 4. Range Tree: Multi-Dimensional Filtering ────────────────────────
    std::cout << "\n--- MODULE 4: Range Tree Multi-Dimensional Filter ---" << std::endl;

    auto hotBright = engine.filterStars(-2.0f, 0.5f, 7000.0f, 15000.0f);
    printStarList(hotBright, "Hot & Bright  (mag in [-2.0, 0.5], temp in [7000, 15000]K)");

    auto coolStars = engine.filterByTemperature(0.0f, 5000.0f);
    printStarList(coolStars, "Cool stars  (temp < 5000 K)");

    auto brightest = engine.filterByMagnitude(-2.0f, 0.0f);
    printStarList(brightest, "Visually brightest  (magnitude < 0)");

    auto sunLike = engine.filterStars(0.0f, 6.0f, 5000.0f, 6500.0f);
    printStarList(sunLike, "Sun-like stars (mag in [0,6], temp in [5000,6500] K)");

    // ── 5. Fibonacci Heap: Event Prioritization ───────────────────────────
    std::cout << "\n--- MODULE 5: Fibonacci Heap Event Queue ---" << std::endl;

    engine.addAstroEvent(1.0, 9,  "URGENT: Betelgeuse supernova precursor detected");
    engine.addAstroEvent(3.5, 7,  "ALERT: Rigel luminosity spike +15%");
    engine.addAstroEvent(2.2, 2,  "WARNING: Sirius companion mass transfer event");
    engine.addAstroEvent(0.5, 3,  "CRITICAL: Canopus X-ray burst");
    engine.addAstroEvent(4.8, 5,  "INFO: Arcturus spectral shift nominal");
    engine.addAstroEvent(1.7, 6,  "ALERT: Vega debris disk asymmetry detected");

    std::cout << "\n  Processing events by priority (lowest = most urgent):\n";
    int count = 1;
    while (engine.hasEvents()) {
        AstroEvent evt = engine.getNextEvent();
        Star* s = engine.lookupById(evt.starId);
        std::cout << "  [" << count++ << "] Priority=" << std::fixed << std::setprecision(1)
                  << evt.priority << "  Star=" << (s ? s->name : "Unknown")
                  << "\n      " << evt.description << std::endl;
    }

    // ── 6. ASCII Star Map ─────────────────────────────────────────────────
    {
        std::cout << "\n--- MODULE 6: ASCII Star Map ---\n\n";

        {
            AstroMap::StarMapConfig cfg;
            cfg.width      = 80;
            cfg.height     = 24;
            cfg.projection = AstroMap::Projection::XY;
            cfg.showGrid   = true;
            cfg.showLabels = true;
            cfg.showLegend = true;
            cfg.zoom       = 1.0f;
            cfg.maxRangeLY = 100.0f;

            AstroMap::AsciiStarMap map(cfg);
            map.loadFromEngine(engine);
            map.print();
        }

        {
            AstroMap::StarMapConfig cfg;
            cfg.width      = 80;
            cfg.height     = 30;
            cfg.projection = AstroMap::Projection::XZ;
            cfg.showGrid   = true;
            cfg.showLabels = true;
            cfg.showLegend = false;
            cfg.zoom       = 1.5f;
            cfg.centerX    = 0.0f;
            cfg.centerY    = 0.0f;
            cfg.maxRangeLY = 700.0f;

            AstroMap::AsciiStarMap map(cfg);
            map.loadFromEngine(engine);
            std::cout << "[XZ edge-on view \xe2\x80\x94 zoom x1.5]\n";
            map.print();
        }
    }

    // ── 7. Benchmark ──────────────────────────────────────────────────────
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "   PERFORMANCE ANALYTICS ENGINE\n";
    std::cout << std::string(60, '=') << "\n";

    AstroMapBenchmark benchmarks;
    std::vector<size_t> scales = {1000, 10000};

    benchmarks.runAll(scales);
    benchmarks.printReport();

    std::cout << "\n[Engine] Analytics complete. CSV exported.\n";
    std::cout << "============================================\n";

    std::vector<std::pair<int, std::string>> curves = {
        {1,"MMMMMMMMMMMMMMMM"}, {2,"HMMMHMMHMMMHMMHM"},
        {3, "MHLMHLMHLMHLMHLM"}, {4, "MMMMMMMMMMMMMMMM"},
        {5, "LMLMLMLMLMLMLMLM"}, {6, "HHHMMMLLLMMMHHHM"},
        {7, "LLLMMMHHHMMMLLLM"}, {8, "MMHMMHMMHMMHMMHM"},
        {9, "HLHLHLHLHLHLHLHL"}, {10, "MHMHMHMHMHMHMHMH"}
    };
    runAdvancedStructuresDemos(curves, engine.getStars(), engine.getOctree());

    std::cout << "\n============================================" << std::endl;
    std::cout << "   All modules executed successfully.        " << std::endl;
    std::cout << "============================================" << std::endl;
    return 0;
}