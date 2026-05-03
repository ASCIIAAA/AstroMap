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

    Star() = default;
    // 1. Identification (For vEB Tree lookup)
    uint64_t id;
    std::string name;

    // 2. Spatial Data (For Octree indexing)
    Vector3D position;

    // 3. Physical Properties
    float magnitude;
    float temperature;
    float mass;

    // 4. Temporal/Spectral Data (For Suffix Tree pattern matching)
    std::string lightCurvePattern;

    // Constructor
    Star(uint64_t id, std::string name, Vector3D pos)
        : id(id), name(std::move(name)), position(pos),
          magnitude(0.0f), temperature(0.0f), mass(0.0f) {}

    double distanceTo(const Star& other) const {
        return position.distanceTo(other.position);
    }
};

#endif