#ifndef VEBTREE_HPP
#define VEBTREE_HPP

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

/**
 * @brief Van Emde Boas Tree for near sub-logarithmic 64-bit integer lookup.
 *
 * This implementation uses a hash-map based approach for the cluster array
 * to handle sparse universes (like star IDs up to UINT64_MAX) without
 * allocating massive arrays. Time complexity: O(log log U).
 */
class vEBTree {
private:
    uint64_t universeSize; // U
    uint64_t minVal;
    uint64_t maxVal;

    static const uint64_t EMPTY = UINT64_MAX;

    std::unique_ptr<vEBTree> summary;
    std::unordered_map<uint64_t, std::unique_ptr<vEBTree>> clusters;

    uint64_t lowerSqrt() const;
    uint64_t upperSqrt() const;
    uint64_t high(uint64_t x) const;
    uint64_t low(uint64_t x) const;
    uint64_t index(uint64_t cluster, uint64_t pos) const;

public:
    explicit vEBTree(uint64_t universeSize);

    void insert(uint64_t x);
    bool contains(uint64_t x) const;
    void remove(uint64_t x);

    uint64_t minimum() const { return minVal; }
    uint64_t maximum() const { return maxVal; }
    bool isEmpty() const { return minVal == EMPTY; }

    uint64_t successor(uint64_t x) const;
    uint64_t predecessor(uint64_t x) const;
};

#endif