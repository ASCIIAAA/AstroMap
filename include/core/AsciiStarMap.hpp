#pragma once
//  AsciiStarMap.hpp  —  ASCII terminal star-map renderer

#include <string>
#include <vector>
#include "engine/GalacticEngine.hpp"        // full definition, no forward decl
#include "structures/Octree.hpp"

namespace AstroMap {

enum class Projection { XY, XZ, YZ };

struct StarMapConfig {
    int        width      = 80;
    int        height     = 24;
    Projection projection = Projection::XY;
    bool       showGrid   = true;
    bool       showLabels = true;
    bool       showLegend = true;
    float      zoom       = 1.0f;
    float      centerX    = 0.0f;
    float      centerY    = 0.0f;
    float      maxRangeLY = 0.0f;
};

class AsciiStarMap {
public:
    explicit AsciiStarMap(const StarMapConfig& cfg = {});

    void        loadFromEngine(const GalacticEngine& engine);
    std::string render() const;
    void        print()  const;

private:
    StarMapConfig        m_cfg;
    std::vector<Star>    m_stars;

    // These are DATA MEMBERS — declared here, NOT inside any function
    float m_minA = 0, m_maxA = 1;
    float m_minB = 0, m_maxB = 1;

    bool        project(const Star& s, int& cx, int& cy) const;
    static char glyphForMag(float mag);
    std::vector<std::string> makeCanvas() const;

    // Declaration only — implementation stays in AsciiStarMap.cpp
    void computeBounds(float& minA, float& maxA,
                       float& minB, float& maxB) const;
};

} // namespace AstroMap