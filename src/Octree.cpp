#include "structures/Octree.hpp"

void Octree::subdivide() {
    double newHalf = boundary.halfDimension / 2.0;
    
    for (int i = 0; i < 8; i++) {
        Vector3D newCenter = boundary.center;
        // Binary offset math to determine child centers
        newCenter.x += (i & 1 ? newHalf : -newHalf);
        newCenter.y += (i & 2 ? newHalf : -newHalf);
        newCenter.z += (i & 4 ? newHalf : -newHalf);

        children[i] = std::make_unique<Octree>(Boundary{newCenter, newHalf});
    }
    isDivided = true;

    // Move current stars into children
    for (Star* s : stars) {
        for (int i = 0; i < 8; i++) {
            if (children[i]->insert(s)) break;
        }
    }
    stars.clear();
}

bool Octree::insert(Star* star) {
    if (!boundary.contains(star->position)) return false;

    if (!isDivided && stars.size() < MAX_STARS_PER_NODE) {
        stars.push_back(star);
        return true;
    }

    if (!isDivided) subdivide();

    for (int i = 0; i < 8; i++) {
        if (children[i]->insert(star)) return true;
    }

    return false;
}

void Octree::queryRange(const Boundary& range, std::vector<Star*>& found) const {
    // Check if range intersects this boundary (simplified for brevity)
    // In a full engine, you'd add an intersects() helper here
    
    if (isDivided) {
        for (int i = 0; i < 8; i++) {
            children[i]->queryRange(range, found);
        }
    } else {
        for (Star* s : stars) {
            if (range.contains(s->position)) {
                found.push_back(s);
            }
        }
    }
}