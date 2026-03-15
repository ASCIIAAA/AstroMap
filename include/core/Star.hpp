#ifndef STAR_HPP
#define STAR_HPP

#include "Vector3D.hpp"
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Represents a single celestial body within the Astro-Map Engine.
 */
struct Star {
    // 1. Identification (For vEB Tree lookup)
    // Using uint64_t to handle "billion-scale" star catalogs
    uint64_t id; 
    std::string name;

    // 2. Spatial Data (For Octree indexing)
    Vector3D position; // Coordinates in Light Years or Parsecs

    // 3. Physical Properties
    float magnitude;    // Apparent brightness
    float temperature;  // Surface temperature in Kelvin
    float mass;         // In Solar Masses

    // 4. Temporal/Spectral Data (For Suffix Tree pattern matching)
    // Points to the light-curve data stored in lightcurves.txt
    std::vector<float> lightCurve; 

    // Constructor
    Star(uint64_t id, std::string name, Vector3D pos)
        : id(id), name(std::move(name)), position(pos), 
          magnitude(0.0f), temperature(0.0f), mass(0.0f) {}

    // Utility: Calculate distance to another star
    double distanceTo(const Star& other) const {
        return position.distanceTo(other.position);
    }
};

#endif