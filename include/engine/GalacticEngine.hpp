#ifndef GALACTICENGINE_HPP
#define GALACTICENGINE_HPP

#include "core/Star.hpp"
#include "structures/Octree.hpp"
#include "structures/vEBTree.hpp"
#include "structures/FibonacciHeap.hpp"
#include "structures/SuffixTree.hpp"
#include "structures/RangeTree.hpp"
#include "benchmark.hpp"

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

/**
 * @brief Central engine that ties all data structures together.
 *
 * Responsibilities:
 *   - Load stars from CSV and light-curves from TXT
 *   - Build and expose all indexes
 *   - Provide high-level query interface
 */
class GalacticEngine {
private:
    std::vector<Star> stars;                          // Owns star objects
    std::unordered_map<uint64_t, Star*> idIndex;      // Fast pointer lookup

    std::unique_ptr<Octree>        spatialIndex;
    std::unique_ptr<vEBTree>       idTree;
    std::unique_ptr<SuffixTree>    patternIndex;
    std::unique_ptr<FibonacciHeap> eventQueue;
    std::unique_ptr<RangeTree>     rangeIndex;

    void buildIndexes();
    void loadLightCurves(const std::string& filename);

public:
    GalacticEngine();
    // Add this to GalacticEngine.hpp inside public:
    const std::vector<Star>& getStars() const { return stars; }

    // Data loading
    bool loadStars(const std::string& csvFile);
    bool loadEvents(const std::string& lightCurveFile);

    // Spatial queries (Octree)
    std::vector<Star*> findStarsInRegion(Vector3D center, double radius) const;
    Star* findNearestStar(const Vector3D& point) const;

    // ID lookup (vEB Tree)
    Star* lookupById(uint64_t id);
    bool  starExists(uint64_t id) const;

    // Pattern matching (Suffix Tree)
    std::vector<Star*> findByLightCurvePattern(const std::string& pattern);

    // Multi-dimensional filtering (Range Tree)
    std::vector<Star*> filterStars(float magLo, float magHi, float tempLo, float tempHi);
    std::vector<Star*> filterByMagnitude(float magLo, float magHi);
    std::vector<Star*> filterByTemperature(float tempLo, float tempHi);

    // Event prioritization (Fibonacci Heap)
    void  addAstroEvent(double priority, uint64_t starId, const std::string& desc);
    AstroEvent getNextEvent();
    bool  hasEvents() const;

    // Stats
    int starCount() const { return (int)stars.size(); }
    void printSummary() const;
    
    const Octree& getOctree() const { return *spatialIndex; }
};

#endif