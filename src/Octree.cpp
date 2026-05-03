#include "structures/Octree.hpp"
#include <array>
#include <queue>

void Octree::subdivide() {
    double newHalf = boundary.halfDimension / 2.0;

    for (int i = 0; i < 8; i++) {
        Vector3D newCenter = boundary.center;
        newCenter.x += (i & 1 ? newHalf : -newHalf);
        newCenter.y += (i & 2 ? newHalf : -newHalf);
        newCenter.z += (i & 4 ? newHalf : -newHalf);
        children[i] = std::make_unique<Octree>(Boundary{newCenter, newHalf});
    }
    isDivided = true;

    // Re-insert existing stars into children.
    // Stars that fall exactly on a child boundary may not fit any child;
    // keep them in overflowStars (separate from stars[]) so queryRange
    // can find them without double-counting.
    for (Star* s : stars) {
        bool placed = false;
        for (int i = 0; i < 8; i++) {
            if (children[i]->insert(s)) { placed = true; break; }
        }
        if (!placed) overflowStars.push_back(s);
    }
    stars.clear();
}

bool Octree::insert(Star* star) {
    if (!boundary.contains(star->position)) return false;

    if (!isDivided && (int)stars.size() < MAX_STARS_PER_NODE) {
        stars.push_back(star);
        return true;
    }

    if (!isDivided) subdivide();

    for (int i = 0; i < 8; i++) {
        if (children[i]->insert(star)) return true;
    }

    // Boundary-edge star: no child claimed it — store in overflow.
    overflowStars.push_back(star);
    return true;
}

void Octree::queryRange(const Boundary& range, std::vector<Star*>& found) const {
    if (!boundary.intersects(range)) return;

    if (isDivided) {
        // Always check overflow stars on internal nodes (boundary-edge cases).
        for (Star* s : overflowStars) {
            if (range.contains(s->position))
                found.push_back(s);
        }
        for (int i = 0; i < 8; i++)
            children[i]->queryRange(range, found);
    } else {
        // Leaf node: all data lives in stars[].
        for (Star* s : stars) {
            if (range.contains(s->position))
                found.push_back(s);
        }
    }
}

int Octree::size() const {
    int count = (int)stars.size() + (int)overflowStars.size();
    if (isDivided) {
        for (int i = 0; i < 8; i++) count += children[i]->size();
    }
    return count;
}

std::vector<std::array<bool, 8>> Octree::getChildMasks() const {
    std::vector<std::array<bool, 8>> masks;
    std::queue<const Octree*> q;
    q.push(this);
    while (!q.empty()) {
        const Octree* node = q.front(); q.pop();
        std::array<bool, 8> mask = {};
        for (int c = 0; c < 8; ++c) {
            mask[c] = (node->children[c] != nullptr);
            if (mask[c]) q.push(node->children[c].get());
        }
        masks.push_back(mask);
    }
    return masks;
}