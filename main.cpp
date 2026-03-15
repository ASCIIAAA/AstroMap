#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "core/Vector3D.hpp"
#include "core/Star.hpp"
#include "structures/Octree.hpp"

// Utility function to load stars from your CSV
std::vector<Star> parseCSV(const std::string& filename) {
    std::vector<Star> starList;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return starList;
    }

    // Skip Header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string idStr, name, xStr, yStr, zStr, magStr, tempStr, massStr;

        std::getline(ss, idStr, ',');
        std::getline(ss, name, ',');
        std::getline(ss, xStr, ',');
        std::getline(ss, yStr, ',');
        std::getline(ss, zStr, ',');
        std::getline(ss, magStr, ',');
        std::getline(ss, tempStr, ',');
        std::getline(ss, massStr, ',');

        if (!idStr.empty()) {
            Vector3D pos(std::stod(xStr), std::stod(yStr), std::stod(zStr));
            Star s(std::stoull(idStr), name, pos);
            s.magnitude = std::stof(magStr);
            starList.push_back(s);
        }
    }
    return starList;
}

int main() {
    std::cout << "--- Astro-Map Galactic Navigation Engine ---" << std::endl;

    // 1. Load Data
    std::vector<Star> allStars = parseCSV("data/stars.csv");
    std::cout << "[Info] Loaded " << allStars.size() << " stars from database." << std::endl;

    // 2. Initialize Octree 
    // We'll define a boundary representing a sector of space (e.g., 1000x1000x1000 light years)
    Boundary galacticSector = { Vector3D(0, 0, 0), 1000.0 };
    Octree spatialIndex(galacticSector);

    // 3. Build the Index
    for (auto& star : allStars) {
        spatialIndex.insert(&star);
    }
    std::cout << "[Info] Spatial indexing complete." << std::endl;

    // 4. Perform a Spatial Query
    // Let's find stars near "Sirius" (which is at x=8.6 in our CSV)
    // We define a search box centered at (10, 0, 0) with a half-size of 5 units
    Boundary searchRegion = { Vector3D(10.0, 0.0, 0.0), 5.0 };
    std::vector<Star*> results;
    spatialIndex.queryRange(searchRegion, results);

    std::cout << "[Query] Found " << results.size() << " stars in search region:" << std::endl;
    for (Star* s : results) {
        std::cout << " -> " << s->name << " at " << s->position << std::endl;
    }

    return 0;
}