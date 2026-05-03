#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "core/Star.hpp"
#include "core/Vector3D.hpp"
#include <vector>
#include <memory>
#include <array>

// Lower threshold forces subdivision on the real 30-star dataset,
// making spatial queries genuinely tree-based and giving the SBV demo
// a real multi-node octree to encode.
const int MAX_STARS_PER_NODE = 4;

struct Boundary {
    Vector3D center;
    double halfDimension;

    bool contains(const Vector3D& point) const {
        return (point.x >= center.x - halfDimension &&
                point.x <= center.x + halfDimension &&
                point.y >= center.y - halfDimension &&
                point.y <= center.y + halfDimension &&
                point.z >= center.z - halfDimension &&
                point.z <= center.z + halfDimension);
    }

    bool intersects(const Boundary& other) const {
        return !(other.center.x - other.halfDimension > center.x + halfDimension ||
                 other.center.x + other.halfDimension < center.x - halfDimension ||
                 other.center.y - other.halfDimension > center.y + halfDimension ||
                 other.center.y + other.halfDimension < center.y - halfDimension ||
                 other.center.z - other.halfDimension > center.z + halfDimension ||
                 other.center.z + other.halfDimension < center.z - halfDimension);
    }
};

class Octree {
private:
    Boundary boundary;

    // Leaf storage: stars that live in this node before subdivision.
    std::vector<Star*> stars;

    // Overflow storage: stars that could not be placed in any child octant
    // after subdivision (boundary-edge cases). Kept separate so queryRange
    // never double-counts them with child results.
    std::vector<Star*> overflowStars;

    std::unique_ptr<Octree> children[8];
    bool isDivided = false;

    void subdivide();

public:
    explicit Octree(Boundary b) : boundary(b) {}

    bool insert(Star* star);
    void queryRange(const Boundary& range, std::vector<Star*>& found) const;
    int  size() const;
    std::vector<std::array<bool, 8>> getChildMasks() const;
    const Octree& getOctree() const { return *this; }
};

#endif