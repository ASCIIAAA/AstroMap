#ifndef RANGETREE_HPP
#define RANGETREE_HPP

#include "core/Star.hpp"
#include <vector>
#include <algorithm>
#include <memory>

/**
 * @brief 2D Range Tree for multi-dimensional star filtering.
 *
 * Primary axis  : magnitude
 * Secondary axis: temperature
 *
 * Query: "find all stars where magnitude in [magLo, magHi]
 *                           AND temperature in [tempLo, tempHi]"
 *
 * Time complexity:
 *   Build : O(n log n)
 *   Query : O(log^2 n + k)  where k = results returned
 *   Space : O(n log n)
 */

// Inner tree on temperature axis
struct TempNode {
    float temp;
    Star* star;
    std::unique_ptr<TempNode> left, right;

    TempNode(float t, Star* s) : temp(t), star(s) {}
};

class TempTree {
private:
    std::unique_ptr<TempNode> root;

    TempNode* insertNode(std::unique_ptr<TempNode>& node, float temp, Star* star);
    void queryRange(TempNode* node, float lo, float hi,
                    std::vector<Star*>& results) const;

public:
    void insert(Star* star);
    void query(float tempLo, float tempHi, std::vector<Star*>& results) const;
};

// Outer tree on magnitude axis
struct MagNode {
    float mag;
    Star* star;
    std::unique_ptr<MagNode> left, right;

    // Each magnitude node also stores a full TempTree of all stars
    // in its subtree (fractional cascading simplified version)
    TempTree subtreeTempIndex;

    MagNode(float m, Star* s) : mag(m), star(s) {}
};

class RangeTree {
private:
    std::unique_ptr<MagNode> root;

    MagNode* insertNode(std::unique_ptr<MagNode>& node, Star* star);
    void queryRange(MagNode* node,
                    float magLo,  float magHi,
                    float tempLo, float tempHi,
                    std::vector<Star*>& results) const;

public:
    // Insert a single star
    void insert(Star* star);

    // Query: magnitude in [magLo, magHi] AND temperature in [tempLo, tempHi]
    std::vector<Star*> query(float magLo,  float magHi,
                             float tempLo, float tempHi) const;

    // Convenience: query only by magnitude
    std::vector<Star*> queryByMagnitude(float magLo, float magHi) const;

    // Convenience: query only by temperature
    std::vector<Star*> queryByTemperature(float tempLo, float tempHi) const;
};

#endif