#include "engine/GalacticEngine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

// Strip \r, \n, spaces from both ends
static void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' '))
        s.pop_back();
    size_t start = s.find_first_not_of(" \t");
    if (start != std::string::npos) s = s.substr(start);
    else if (!s.empty()) s.clear();
}

GalacticEngine::GalacticEngine() {
    idTree       = std::make_unique<vEBTree>(1 << 20);
    eventQueue   = std::make_unique<FibonacciHeap>();
    patternIndex = std::make_unique<SuffixTree>();
    rangeIndex   = std::make_unique<RangeTree>();

    // boundary was halfDimension=2000, but Deneb is at x=2600 — outside.
    // Increased to 3000 so all 30 CSV stars are contained.
    // Rule of thumb: set this to max(|x|, |y|, |z|) across all stars + 10% margin.
    Boundary galacticBound = { Vector3D(0, 0, 0), 3000.0 };
    spatialIndex = std::make_unique<Octree>(galacticBound);
}

bool GalacticEngine::loadStars(const std::string& csvFile) {
    stars.clear();
    stars.reserve(1000);
    std::ifstream file(csvFile);
    if (!file.is_open()) {
        std::cerr << "[Error] Cannot open: " << csvFile << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string idStr, name, xStr, yStr, zStr, magStr, tempStr, massStr;

        std::getline(ss, idStr,   ',');
        std::getline(ss, name,    ',');
        std::getline(ss, xStr,    ',');
        std::getline(ss, yStr,    ',');
        std::getline(ss, zStr,    ',');
        std::getline(ss, magStr,  ',');
        std::getline(ss, tempStr, ',');
        std::getline(ss, massStr);   // last field, no trailing comma

        trim(idStr); trim(name); trim(xStr); trim(yStr); trim(zStr);
        trim(magStr); trim(tempStr); trim(massStr);

        if (idStr.empty() || xStr.empty()) continue;

        try {
            Vector3D pos(std::stod(xStr), std::stod(yStr), std::stod(zStr));
            Star s(std::stoull(idStr), name, pos);
            s.magnitude   = std::stof(magStr);
            s.temperature = std::stof(tempStr);
            s.mass        = std::stof(massStr);
            stars.push_back(std::move(s));
        } catch (const std::exception& e) {
            std::cerr << "[Warning] Skipping bad line: \"" << line
                      << "\" (" << e.what() << ")" << std::endl;
        }
    }

    buildIndexes();
    std::cout << "[Engine] Loaded " << stars.size() << " stars." << std::endl;
    return true;
}

void GalacticEngine::buildIndexes() {
    for (auto& star : stars) {
        spatialIndex->insert(&star);
        idIndex[star.id] = &star;
        idTree->insert(star.id);
        rangeIndex->insert(&star);
    }
}

bool GalacticEngine::loadEvents(const std::string& lightCurveFile) {
    std::ifstream file(lightCurveFile);
    if (!file.is_open()) {
        std::cerr << "[Error] Cannot open: " << lightCurveFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string idPart  = line.substr(0, colon);
        std::string pattern = line.substr(colon + 1);
        trim(idPart); trim(pattern);
        if (idPart.empty() || pattern.empty()) continue;

        uint64_t starId = std::stoull(idPart);
        auto it = idIndex.find(starId);
        if (it != idIndex.end()) it->second->lightCurvePattern = pattern;
        patternIndex->addPattern(starId, pattern);
    }

    std::cout << "[Engine] Light-curve patterns indexed." << std::endl;
    return true;
}

std::vector<Star*> GalacticEngine::findStarsInRegion(Vector3D center, double radius) const {
    Boundary region = { center, radius };
    std::vector<Star*> results;
    spatialIndex->queryRange(region, results);
    return results;
}

Star* GalacticEngine::findNearestStar(const Vector3D& point) const {
    double searchRadius = 10.0;
    while (searchRadius < 5000.0) {
        auto candidates = findStarsInRegion(point, searchRadius);
        if (!candidates.empty()) {
            Star* nearest = nullptr;
            double minDist = std::numeric_limits<double>::max();
            for (Star* s : candidates) {
                double d = s->position.distanceTo(point);
                if (d < minDist) { minDist = d; nearest = s; }
            }
            return nearest;
        }
        searchRadius *= 2.0;
    }
    return nullptr;
}

Star* GalacticEngine::lookupById(uint64_t id) {
    if (!idTree->contains(id)) return nullptr;
    auto it = idIndex.find(id);
    return (it != idIndex.end()) ? it->second : nullptr;
}

bool GalacticEngine::starExists(uint64_t id) const {
    return idTree->contains(id);
}

std::vector<Star*> GalacticEngine::findByLightCurvePattern(const std::string& pattern) {
    std::vector<int> ids = patternIndex->search(pattern);
    std::vector<Star*> result;
    for (int id : ids) {
        auto it = idIndex.find((uint64_t)id);
        if (it != idIndex.end()) result.push_back(it->second);
    }
    return result;
}

std::vector<Star*> GalacticEngine::filterStars(float magLo, float magHi, float tempLo, float tempHi){
    return rangeIndex->query(magLo, magHi, tempLo, tempHi);
}

std::vector<Star*> GalacticEngine::filterByMagnitude(float magLo, float magHi){
    return rangeIndex->queryByMagnitude(magLo, magHi);
}

std::vector<Star*> GalacticEngine::filterByTemperature(float tempLo, float tempHi){
    return rangeIndex->queryByTemperature(tempLo, tempHi);
}

void GalacticEngine::addAstroEvent(double priority, uint64_t starId, const std::string& desc) {
    eventQueue->insert({priority, starId, desc});
}

AstroEvent GalacticEngine::getNextEvent() {
    return eventQueue->extractMin();
}

bool GalacticEngine::hasEvents() const {
    return !eventQueue->isEmpty();
}

void GalacticEngine::printSummary() const {
    std::cout << "\n=== Astro-Map Engine Summary ===" << std::endl;
    std::cout << "  Stars loaded     : " << stars.size()         << std::endl;
    std::cout << "  Octree size      : " << spatialIndex->size() << std::endl;
    std::cout << "  vEB min ID       : " << idTree->minimum()    << std::endl;
    std::cout << "  vEB max ID       : " << idTree->maximum()    << std::endl;
    std::cout << "  Pending events   : " << eventQueue->size()   << std::endl;
    std::cout << "================================\n"             << std::endl;
}