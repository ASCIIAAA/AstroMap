#include "structures/vEBTree.hpp"
#include <cmath>
#include <stdexcept>

vEBTree::vEBTree(uint64_t u) : universeSize(u), minVal(EMPTY), maxVal(EMPTY) {
    if (u <= 2) return; // Base case: no children needed

    uint64_t upper = upperSqrt();
    summary = std::make_unique<vEBTree>(upper);
    // Clusters are created lazily in the unordered_map
}

uint64_t vEBTree::lowerSqrt() const {
    return static_cast<uint64_t>(std::pow(2.0, std::floor(std::log2(universeSize) / 2.0)));
}

uint64_t vEBTree::upperSqrt() const {
    return static_cast<uint64_t>(std::pow(2.0, std::ceil(std::log2(universeSize) / 2.0)));
}

uint64_t vEBTree::high(uint64_t x) const {
    return x / lowerSqrt();
}

uint64_t vEBTree::low(uint64_t x) const {
    return x % lowerSqrt();
}

uint64_t vEBTree::index(uint64_t cluster, uint64_t pos) const {
    return cluster * lowerSqrt() + pos;
}

void vEBTree::insert(uint64_t x) {
    if (x >= universeSize) return;

    if (minVal == EMPTY) {
        minVal = maxVal = x;
        return;
    }

    if (x == minVal || x == maxVal) return; // Already present

    if (x < minVal) std::swap(x, minVal);
    if (x > maxVal) maxVal = x;

    if (universeSize <= 2) return;

    uint64_t h = high(x);
    uint64_t l = low(x);
    uint64_t lower = lowerSqrt();
    uint64_t upper = upperSqrt();

    if (clusters.find(h) == clusters.end()) {
        clusters[h] = std::make_unique<vEBTree>(lower);
    }

    if (clusters[h]->isEmpty()) {
        summary->insert(h);
    }
    clusters[h]->insert(l);
}

bool vEBTree::contains(uint64_t x) const {
    if (x == minVal || x == maxVal) return true;
    if (universeSize <= 2) return false;

    uint64_t h = high(x);
    auto it = clusters.find(h);
    if (it == clusters.end()) return false;
    return it->second->contains(low(x));
}

void vEBTree::remove(uint64_t x) {
    if (minVal == maxVal) { // Only one element
        if (minVal == x) minVal = maxVal = EMPTY;
        return;
    }

    if (universeSize <= 2) {
        if (x == 0) minVal = 1;
        else maxVal = 0;
        if (minVal > maxVal) minVal = maxVal = EMPTY;
        return;
    }

    if (x == minVal) {
        uint64_t firstCluster = summary->minimum();
        if (firstCluster == EMPTY) { minVal = maxVal = EMPTY; return; }
        x = index(firstCluster, clusters[firstCluster]->minimum());
        minVal = x;
    }

    uint64_t h = high(x);
    auto it = clusters.find(h);
    if (it != clusters.end()) {
        it->second->remove(low(x));
        if (it->second->isEmpty()) {
            summary->remove(h);
        }
    }

    if (x == maxVal) {
        uint64_t summaryMax = summary->maximum();
        if (summaryMax == EMPTY) {
            maxVal = minVal;
        } else {
            maxVal = index(summaryMax, clusters[summaryMax]->maximum());
        }
    }
}

uint64_t vEBTree::successor(uint64_t x) const {
    if (universeSize <= 2) {
        if (x == 0 && maxVal == 1) return 1;
        return EMPTY;
    }
    if (minVal != EMPTY && x < minVal) return minVal;

    uint64_t h = high(x);
    uint64_t l = low(x);

    auto it = clusters.find(h);
    uint64_t maxLow = EMPTY;
    if (it != clusters.end()) maxLow = it->second->maximum();

    if (maxLow != EMPTY && l < maxLow) {
        return index(h, it->second->successor(l));
    } else {
        uint64_t succCluster = summary->successor(h);
        if (succCluster == EMPTY) return EMPTY;
        auto cit = clusters.find(succCluster);
        if (cit == clusters.end()) return EMPTY;
        return index(succCluster, cit->second->minimum());
    }
}

uint64_t vEBTree::predecessor(uint64_t x) const {
    if (universeSize <= 2) {
        if (x == 1 && minVal == 0) return 0;
        return EMPTY;
    }
    if (maxVal != EMPTY && x > maxVal) return maxVal;

    uint64_t h = high(x);
    uint64_t l = low(x);

    auto it = clusters.find(h);
    uint64_t minLow = EMPTY;
    if (it != clusters.end()) minLow = it->second->minimum();

    if (minLow != EMPTY && l > minLow) {
        return index(h, it->second->predecessor(l));
    } else {
        uint64_t predCluster = summary->predecessor(h);
        if (predCluster == EMPTY) {
            if (minVal != EMPTY && x > minVal) return minVal;
            return EMPTY;
        }
        auto cit = clusters.find(predCluster);
        if (cit == clusters.end()) return EMPTY;
        return index(predCluster, cit->second->maximum());
    }
}