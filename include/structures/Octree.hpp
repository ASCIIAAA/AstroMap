#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "core/Star.hpp"
#include "core/Vector3D.hpp"
#include <vector>
#include <memory>

// Constants for tuning performance
const int MAX_STARS_PER_NODE = 100;

struct Boundary {
    Vector3D center;
    double halfDimension;

    bool contains(const Vector3D& point) const {
        return (point.x >= center.x - halfDimension && point.x <= center.x + halfDimension &&
                point.y >= center.y - halfDimension && point.y <= center.y + halfDimension &&
                point.z >= center.z - halfDimension && point.z <= center.z + halfDimension);
    }
};

class Octree {
private:
    Boundary boundary;
    std::vector<Star*> stars; // Stars stored in this node

    // Children: 0: -x-y-z, 1: +x-y-z, 2: -x+y-z, 3: +x+y-z...
    std::unique_ptr<Octree> children[8];
    bool isDivided = false;

    void subdivide();

public:
    Octree(Boundary b) : boundary(b) {}

    bool insert(Star* star);
    void queryRange(const Boundary& range, std::vector<Star*>& found) const;
};

#endif