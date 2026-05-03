#include "structures/RangeTree.hpp"
#include <limits>

//  TempTree  (inner BST on temperature)

TempNode* TempTree::insertNode(std::unique_ptr<TempNode>& node,
                                float temp, Star* star) {
    if (!node) {
        node = std::make_unique<TempNode>(temp, star);
        return node.get();
    }
    if (temp <= node->temp)
        insertNode(node->left,  temp, star);
    else
        insertNode(node->right, temp, star);
    return node.get();
}

void TempTree::insert(Star* star) {
    insertNode(root, star->temperature, star);
}

// standard BST range traversal — each node visited and recorded once.
void TempTree::queryRange(TempNode* node, float lo, float hi,
                           std::vector<Star*>& results) const {
    if (!node) return;

    // Left subtree may have temps in [lo, hi] if node->temp > lo.
    if (node->temp > lo)
        queryRange(node->left.get(), lo, hi, results);

    // Record this node if it is in range.
    if (node->temp >= lo && node->temp <= hi)
        results.push_back(node->star);

    // Right subtree may have temps in [lo, hi] if node->temp < hi.
    if (node->temp < hi)
        queryRange(node->right.get(), lo, hi, results);
}

void TempTree::query(float tempLo, float tempHi,
                      std::vector<Star*>& results) const {
    queryRange(root.get(), tempLo, tempHi, results);
}

//  RangeTree  (outer BST on magnitude)

MagNode* RangeTree::insertNode(std::unique_ptr<MagNode>& node, Star* star) {
    if (!node) {
        node = std::make_unique<MagNode>(star->magnitude, star);
        node->subtreeTempIndex.insert(star);
        return node.get();
    }

    // Every ancestor accumulates this star in its subtree temp index.
    node->subtreeTempIndex.insert(star);

    if (star->magnitude <= node->mag)
        insertNode(node->left,  star);
    else
        insertNode(node->right, star);

    return node.get();
}

void RangeTree::insert(Star* star) {
    insertNode(root, star);
}

// standard BST range traversal on the magnitude axis.
// Each node is visited at most once, and node->star is recorded at most once.
void RangeTree::queryRange(MagNode* node,
                            float magLo,  float magHi,
                            float tempLo, float tempHi,
                            std::vector<Star*>& results) const {
    if (!node) return;

    // Left subtree may contain mags in [magLo, magHi] if node->mag > magLo.
    if (node->mag > magLo)
        queryRange(node->left.get(), magLo, magHi, tempLo, tempHi, results);

    // Record this node if both axes are in range.
    if (node->mag >= magLo && node->mag <= magHi) {
        if (node->star->temperature >= tempLo &&
            node->star->temperature <= tempHi) {
            results.push_back(node->star);
        }
    }

    // Right subtree may contain mags in [magLo, magHi] if node->mag < magHi.
    if (node->mag < magHi)
        queryRange(node->right.get(), magLo, magHi, tempLo, tempHi, results);
}

std::vector<Star*> RangeTree::query(float magLo,  float magHi,
                                     float tempLo, float tempHi) const {
    std::vector<Star*> results;
    queryRange(root.get(), magLo, magHi, tempLo, tempHi, results);
    return results;
}

std::vector<Star*> RangeTree::queryByMagnitude(float magLo, float magHi) const {
    return query(magLo, magHi,
                 -std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max());
}

std::vector<Star*> RangeTree::queryByTemperature(float tempLo, float tempHi) const {
    return query(-std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max(),
                  tempLo, tempHi);
}