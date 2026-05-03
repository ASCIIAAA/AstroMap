#ifndef ADVANCED_STRUCTURES_HPP
#define ADVANCED_STRUCTURES_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

template<typename T>
struct Optional {
    bool hasValue;
    T    value;
    Optional()          : hasValue(false), value() {}
    Optional(const T& v): hasValue(true),  value(v) {}
    bool has_value() const { return hasValue; }
    T*       operator->()       { return &value; }
    const T* operator->() const { return &value; }
    T&       operator*()        { return value; }
    const T& operator*()  const { return value; }
    explicit operator bool() const { return hasValue; }
};
template<typename T>
Optional<T> make_optional(const T& v) { return Optional<T>(v); }
struct NullOpt {};
// inline NullOpt nullopt;
template<typename T>
bool operator!(const Optional<T>& o) { return !o.hasValue; }

// These two are already guarded with #pragma once / include guards
#include "core/Star.hpp"
#include "structures/Octree.hpp"

//  1.  HUFFMAN COMPRESSION

struct HuffmanNode {
    char     ch   = '\0';
    uint64_t freq = 0;
    std::unique_ptr<HuffmanNode> left, right;
    bool isLeaf() const { return !left && !right; }
};

struct HuffmanResult {
    std::string          original;
    std::vector<uint8_t> compressedBytes;
    size_t               bitLength = 0;
    std::unordered_map<char, std::string> codeTable;

    double compressionRatio()   const;
    double spaceSavingPercent() const;
    void   printReport()        const;
};

class HuffmanCompressor {
public:
    HuffmanResult compress  (const std::string& input)   const;
    std::string   decompress(const HuffmanResult& result) const;

    // Compress all light curves and print a comparison table.
    // Returns the average ratio across all curves.
    double compressAll(const std::vector<std::pair<int, std::string>>& curves) const;

private:
    std::unique_ptr<HuffmanNode> buildTree(
        const std::unordered_map<char, uint64_t>& freq) const;

    void buildCodes(const HuffmanNode* node, const std::string& prefix,
                    std::unordered_map<char, std::string>& table) const;

    std::vector<uint8_t> packBits  (const std::string& bits, size_t& bitLen) const;
    std::string          unpackBits(const std::vector<uint8_t>& bytes, size_t bitLen) const;
};


//  2.  B-TREE DISK INDEX

static constexpr size_t BTREE_PAGE_SIZE_BYTES = 4096; // 4 KB page

// Compact record stored per star in the leaf pages.
struct StarRecord {
    uint64_t id          = 0;
    float    magnitude   = 0.f;
    float    temperature = 0.f;
    char     name[32]    = {};
};

// Non-template node avoids cross-TU linkage issues with MSVC / older GCC.
struct BTreeNode {
    bool isLeaf = true;
    int  _T     = 2;
    std::vector<uint64_t>    keys;
    std::vector<StarRecord>  values;      // leaf nodes only
    std::vector<std::shared_ptr<BTreeNode>> children;

    BTreeNode(int t, bool leaf) : isLeaf(leaf), _T(t) {}
};

class BTreeStarIndex {
public:
    explicit BTreeStarIndex(size_t pageSizeBytes = BTREE_PAGE_SIZE_BYTES);

    void insert(const StarRecord& rec);

    // Returns the record (if found) and increments diskReads per page visited.
    Optional<StarRecord> search(uint64_t id, int& diskReads) const;

    void buildIndex(const std::vector<Star>& stars);

    // Print a side-by-side B-Tree vs BST disk-read comparison for one query.
    void printVsBSTComparison(uint64_t queryId) const;

    int  tDegree()   const { return _t; }
    int  height()    const;
    long nodeCount() const { return _nodeCount; }

private:
    using NodePtr = std::shared_ptr<BTreeNode>;

    int     _t;
    NodePtr _root;
    long    _nodeCount = 0;
    size_t  _pageSize;

    void splitChild   (NodePtr parent, int i, NodePtr child);
    void insertNonFull(NodePtr node, const StarRecord& rec);

    Optional<StarRecord> searchNode(const NodePtr& node,
                                 uint64_t id, int& reads) const;
    int  calcHeight(const NodePtr& node) const;
    long countNodes(const NodePtr& node) const;
    long _starCount = 0;
};

//  3.  SUCCINCT BIT VECTOR  (LOUDS encoding of the Octree)

class SuccinctBitVector {
public:
    // Build from BFS child-presence masks (one 8-element array per Octree node).
    void encode(const std::vector<std::array<bool, 8>>& childMasks);

    uint64_t rank1  (uint64_t i) const;   // # of 1-bits in [0..i]
    uint64_t select1(uint64_t k) const;   // position of the k-th 1-bit
    int64_t  child  (uint64_t nodePos, int octant) const; // -1 if absent

    size_t bitCount()        const { return _bitCount; }
    size_t rawMemoryBytes()  const { return (_bitCount + 7) / 8; }
    size_t rankStructBytes() const { return _rankCache.size() * sizeof(uint64_t); }

    void printMemoryComparison(size_t pointerBasedBytes) const;
    void printBitString(size_t maxBits = 64) const;

private:
    std::vector<uint8_t>  _packed;
    std::vector<uint64_t> _rankCache;
    size_t                _bitCount = 0;

    void buildRankCache();
    bool getBit(uint64_t i) const;
    void setBit(uint64_t i, bool v);
};

// Bridges between your existing Octree and the SuccinctBitVector.
// Requires Octree to expose:  getChildMasks() const
class SuccinctOctreeEncoder {
public:
    static SuccinctBitVector encode(const Octree& tree,
                                    size_t& outPointerBasedBytes);
};

//  DEMO ENTRY POINT

void runAdvancedStructuresDemos(
    const std::vector<std::pair<int, std::string>>& lightCurves,
    const std::vector<Star>& stars,
    const Octree& octree);

#endif