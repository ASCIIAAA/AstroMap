#include "structures/advanced_structures.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <sstream>

//  1.  HUFFMAN COMPRESSION

double HuffmanResult::compressionRatio() const {
    size_t orig = original.size() * 8;
    if (bitLength == 0) return 0.0;
    return static_cast<double>(orig) / static_cast<double>(bitLength);
}

double HuffmanResult::spaceSavingPercent() const {
    double r = compressionRatio();
    return (r <= 0.0) ? 0.0 : (1.0 - 1.0 / r) * 100.0;
}

void HuffmanResult::printReport() const {
    std::cout << "\n  [Huffman] Input      : \""
              << original.substr(0, 32)
              << (original.size() > 32 ? "..." : "") << "\"\n";
    std::cout << "  [Huffman] Char count : " << original.size() << "\n";
    std::cout << "  [Huffman] Code table :\n";

    std::vector<std::pair<char, std::string>> sorted(codeTable.begin(), codeTable.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b){ return a.second.size() < b.second.size(); });

    for (const auto& kv : sorted) {
        std::string label = (kv.first == '\0') ? "EOF" : std::string(1, kv.first);
        std::cout << "      '" << label << "'  ->  " << kv.second << "\n";
    }

    std::cout << "  [Huffman] Original   : " << (original.size() * 8) << " bits\n";
    std::cout << "  [Huffman] Compressed : " << bitLength
              << " bits  (" << compressedBytes.size() << " bytes)\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  [Huffman] Ratio      : " << compressionRatio() << "x\n";
    std::cout << "  [Huffman] Space saved: " << spaceSavingPercent() << "%\n";
}

// Tree construction

std::unique_ptr<HuffmanNode>
HuffmanCompressor::buildTree(const std::unordered_map<char, uint64_t>& freq) const {
    // Custom comparator for the min-heap
    struct Cmp {
        bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
            return a->freq > b->freq;
        }
    };
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Cmp> pq;

    for (const auto& kv : freq) {
        HuffmanNode* n = new HuffmanNode();
        n->ch   = kv.first;
        n->freq = kv.second;
        pq.push(n);
    }

    // Edge case: single unique character
    if (pq.size() == 1) {
        HuffmanNode* parent = new HuffmanNode();
        parent->freq  = pq.top()->freq;
        parent->left  = std::unique_ptr<HuffmanNode>(pq.top()); pq.pop();
        return std::unique_ptr<HuffmanNode>(parent);
    }

    while (pq.size() > 1) {
        HuffmanNode* l = pq.top(); pq.pop();
        HuffmanNode* r = pq.top(); pq.pop();
        HuffmanNode* m = new HuffmanNode();
        m->freq  = l->freq + r->freq;
        m->left  = std::unique_ptr<HuffmanNode>(l);
        m->right = std::unique_ptr<HuffmanNode>(r);
        pq.push(m);
    }
    return std::unique_ptr<HuffmanNode>(pq.top());
}

void HuffmanCompressor::buildCodes(const HuffmanNode* node,
                                    const std::string& prefix,
                                    std::unordered_map<char, std::string>& table) const {
    if (!node) return;
    if (node->isLeaf()) {
        table[node->ch] = prefix.empty() ? "0" : prefix;
        return;
    }
    buildCodes(node->left.get(),  prefix + "0", table);
    buildCodes(node->right.get(), prefix + "1", table);
}

// Bit packing

std::vector<uint8_t> HuffmanCompressor::packBits(const std::string& bitString,
                                                    size_t& bitLen) const {
    bitLen = bitString.size();
    std::vector<uint8_t> bytes((bitLen + 7) / 8, 0);
    for (size_t i = 0; i < bitLen; ++i) {
        if (bitString[i] == '1')
            bytes[i / 8] |= static_cast<uint8_t>(1u << (7 - (i % 8)));
    }
    return bytes;
}

std::string HuffmanCompressor::unpackBits(const std::vector<uint8_t>& bytes,
                                           size_t bitLen) const {
    std::string s;
    s.reserve(bitLen);
    for (size_t i = 0; i < bitLen; ++i)
        s += ((bytes[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0';
    return s;
}

// Public API

HuffmanResult HuffmanCompressor::compress(const std::string& input) const {
    HuffmanResult result;
    result.original = input;
    if (input.empty()) return result;

    std::unordered_map<char, uint64_t> freq;
    for (char c : input) freq[c]++;

    auto root = buildTree(freq);
    buildCodes(root.get(), "", result.codeTable);

    std::string bitString;
    bitString.reserve(input.size() * 4);
    for (char c : input) bitString += result.codeTable.at(c);

    result.compressedBytes = packBits(bitString, result.bitLength);
    return result;
}

std::string HuffmanCompressor::decompress(const HuffmanResult& result) const {
    if (result.original.empty()) return "";

    std::unordered_map<std::string, char> rev;
    for (const auto& kv : result.codeTable) rev[kv.second] = kv.first;

    std::string bits    = unpackBits(result.compressedBytes, result.bitLength);
    std::string decoded, cur;
    for (char bit : bits) {
        cur += bit;
        auto it = rev.find(cur);
        if (it != rev.end()) { decoded += it->second; cur.clear(); }
    }
    return decoded;
}

double HuffmanCompressor::compressAll(
    const std::vector<std::pair<int, std::string>>& curves) const {

    std::cout << "\n--- FEATURE 4: Huffman Light-Curve Compression ---\n";
    std::cout << std::string(72, '-') << "\n";
    std::cout << std::left
              << std::setw(8)  << "StarID"
              << std::setw(16) << "Len(chars)"
              << std::setw(14) << "Orig(bits)"
              << std::setw(18) << "Compressed(bits)"
              << std::setw(10) << "Ratio"
              << "Saved(%)\n";
    std::cout << std::string(72, '-') << "\n";

    double totalRatio = 0.0;
    for (const auto& kv : curves) {
        int id               = kv.first;
        const std::string& p = kv.second;
        HuffmanResult res    = compress(p);
        double ratio         = res.compressionRatio();
        totalRatio          += ratio;

        std::cout << std::left
                  << std::setw(8)  << id
                  << std::setw(16) << p.size()
                  << std::setw(14) << (p.size() * 8)
                  << std::setw(18) << res.bitLength
                  << std::fixed << std::setprecision(2)
                  << std::setw(10) << ratio
                  << res.spaceSavingPercent() << "\n";
    }
    std::cout << std::string(72, '-') << "\n";

    double avg = curves.empty() ? 0.0 : totalRatio / static_cast<double>(curves.size());
    std::cout << "  Average compression ratio : " << avg << "x\n";

    if (!curves.empty()) {
        std::cout << "\n  [Detailed report — Star ID " << curves[0].first << "]\n";
        HuffmanResult r = compress(curves[0].second);
        r.printReport();
        std::string dec = decompress(r);
        std::cout << "  [Huffman] Round-trip check : "
                  << (dec == curves[0].second ? "PASS" : "FAIL") << "\n";
    }
    return avg;
}


//  2.  B-TREE DISK INDEX

// T from page size:  (2T-1)*56 + 2T*16 <= pageBytes  =>  T = (page+56)/144
static int calcDegree(size_t pageBytes) {
    int t = static_cast<int>((pageBytes + 56) / 144);
    return std::max(t, 2);
}

BTreeStarIndex::BTreeStarIndex(size_t pageSizeBytes)
    : _t(calcDegree(pageSizeBytes)),
      _root(std::make_shared<BTreeNode>(_t, true)),
      _nodeCount(1),
      _pageSize(pageSizeBytes) {

    std::cout << "  [B-Tree] Page = " << pageSizeBytes << " B"
              << "  T = " << _t
              << "  max keys/node = " << (2 * _t - 1) << "\n";
}

void BTreeStarIndex::splitChild(NodePtr parent, int i, NodePtr child) {
    int T        = _t;
    auto sibling = std::make_shared<BTreeNode>(T, child->isLeaf);

    sibling->keys.assign(child->keys.begin() + T, child->keys.end());
    if (child->isLeaf)
        sibling->values.assign(child->values.begin() + T, child->values.end());
    else
        sibling->children.assign(child->children.begin() + T, child->children.end());

    uint64_t midKey = child->keys[T - 1];

    child->keys.resize(T - 1);
    if (child->isLeaf) child->values.resize(T - 1);
    else               child->children.resize(T);

    parent->keys.insert(parent->keys.begin() + i, midKey);
    if (!parent->isLeaf || parent->values.empty())
        ; // internal node — values live only in leaves
    parent->children.insert(parent->children.begin() + i + 1, sibling);
    _nodeCount++;
}

void BTreeStarIndex::insertNonFull(NodePtr node, const StarRecord& rec) {
    int i = static_cast<int>(node->keys.size()) - 1;

    if (node->isLeaf) {
        node->keys.push_back(0);
        node->values.push_back(StarRecord{});
        while (i >= 0 && rec.id < node->keys[i]) {
            node->keys[i + 1]   = node->keys[i];
            node->values[i + 1] = node->values[i];
            --i;
        }
        node->keys[i + 1]   = rec.id;
        node->values[i + 1] = rec;
    } else {
        while (i >= 0 && rec.id < node->keys[i]) --i;
        ++i;
        if (static_cast<int>(node->children[i]->keys.size()) == 2 * _t - 1) {
            splitChild(node, i, node->children[i]);
            if (rec.id > node->keys[i]) ++i;
        }
        insertNonFull(node->children[i], rec);
    }
}

void BTreeStarIndex::insert(const StarRecord& rec) {
    if (static_cast<int>(_root->keys.size()) == 2 * _t - 1) {
        auto newRoot = std::make_shared<BTreeNode>(_t, false);
        newRoot->children.push_back(_root);
        _root = newRoot;
        _nodeCount++;
        splitChild(_root, 0, _root->children[0]);
    }
    insertNonFull(_root, rec);
}

Optional<StarRecord>
BTreeStarIndex::searchNode(const NodePtr& node, uint64_t id, int& reads) const {
    reads++;
    int i = 0;
    int n = static_cast<int>(node->keys.size());
    while (i < n && id > node->keys[i]) ++i;

    if (i < n && id == node->keys[i]) {
        if (node->isLeaf) return node->values[i];
        // value is in a leaf — keep descending
    }
    if (node->isLeaf) return Optional<StarRecord>();
    return searchNode(node->children[i], id, reads);
}

Optional<StarRecord>
BTreeStarIndex::search(uint64_t id, int& diskReads) const {
    diskReads = 0;
    return searchNode(_root, id, diskReads);
}

void BTreeStarIndex::buildIndex(const std::vector<Star>& stars) {
     _root       = std::make_shared<BTreeNode>(_t, true);
     _nodeCount  = 1;
     _starCount  = 0;                          // ← reset
     for (const auto& s : stars) {
         StarRecord rec;
         rec.id          = s.id;
         rec.magnitude   = s.magnitude;
         rec.temperature = s.temperature;
         std::strncpy(rec.name, s.name.c_str(), 31);
         rec.name[31] = '\0';
         insert(rec);
         ++_starCount;                         // ← track
     }
}

int BTreeStarIndex::calcHeight(const NodePtr& node) const {
    if (!node || node->isLeaf) return 1;
    return 1 + calcHeight(node->children[0]);
}
int  BTreeStarIndex::height()    const { return calcHeight(_root); }
long BTreeStarIndex::countNodes(const NodePtr& node) const {
    if (!node) return 0;
    long c = 1;
    for (const auto& ch : node->children) c += countNodes(ch);
    return c;
}

void BTreeStarIndex::printVsBSTComparison(uint64_t queryId) const {
     int  btreeReads = 0;
     auto rec        = search(queryId, btreeReads);

     // BST estimate: one node per key, balanced → depth ≈ log2(starCount).
     int bstReads = (_starCount > 0)
                    ? static_cast<int>(std::ceil(std::log2(
                          static_cast<double>(_starCount))))
                    : 0;

     std::cout << "\n--- FEATURE 5: B-Tree vs BST — Query ID " << queryId << " ---\n";
     std::cout << std::string(52, '-') << "\n";
     std::cout << "  Stars indexed      : " << _starCount << "\n";   // ← fixed
     std::cout << "  B-Tree degree (T)  : " << _t << "\n";
     std::cout << "  B-Tree height      : " << height() << "\n";
     std::cout << "  Page size          : " << _pageSize << " bytes\n";
     std::cout << std::string(52, '-') << "\n";
     if (rec) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Found              : " << rec->name
                   << "  mag=" << rec->magnitude
                   << "  temp=" << rec->temperature << "\n";
     } else {
         std::cout << "  Result             : NOT FOUND\n";
     }
     std::cout << "  B-Tree disk reads  : " << btreeReads
               << "  (one page per level)\n";
     std::cout << "  BST disk reads     : ~" << bstReads
               << "  (one node per pointer chase)\n";
     int saved = std::max(0, bstReads - btreeReads);
     std::cout << "  Reads saved        : " << saved
               << std::fixed << std::setprecision(1)
               << "  (" << (bstReads > 0
                              ? 100.0 * saved / bstReads
                              : 0.0)
               << "% fewer)\n";
     std::cout << "  Insight: " << (2 * _t - 1) << " keys/page → only "
               << height() << " page read(s) vs ~" << bstReads
               << " BST node loads\n";
}


//  3.  SUCCINCT BIT VECTOR

bool SuccinctBitVector::getBit(uint64_t i) const {
    return (_packed[i / 8] >> (7 - (i % 8))) & 1u;
}

void SuccinctBitVector::setBit(uint64_t i, bool v) {
    if (v) _packed[i / 8] |=  static_cast<uint8_t>(1u << (7 - (i % 8)));
    else   _packed[i / 8] &= ~static_cast<uint8_t>(1u << (7 - (i % 8)));
}

void SuccinctBitVector::encode(const std::vector<std::array<bool, 8>>& childMasks) {
    _bitCount = childMasks.size() * 8;
    _packed.assign((_bitCount + 7) / 8, 0);

    size_t pos = 0;
    for (const auto& mask : childMasks)
        for (int c = 0; c < 8; ++c, ++pos)
            setBit(pos, mask[c]);

    buildRankCache();
}

void SuccinctBitVector::buildRankCache() {
    size_t blocks = (_bitCount + 511) / 512;
    _rankCache.resize(blocks + 1, 0);
    uint64_t cum = 0;
    for (size_t b = 0; b < blocks; ++b) {
        _rankCache[b] = cum;
        size_t end = std::min((b + 1) * 512, _bitCount);
        for (size_t i = b * 512; i < end; ++i)
            if (getBit(i)) ++cum;
    }
    _rankCache[blocks] = cum;
}

uint64_t SuccinctBitVector::rank1(uint64_t i) const {
    if (i >= _bitCount) return _rankCache.back();
    size_t   block = i / 512;
    uint64_t count = _rankCache[block];
    for (uint64_t j = block * 512; j <= i; ++j)
        if (getBit(j)) ++count;
    return count;
}

uint64_t SuccinctBitVector::select1(uint64_t k) const {
    size_t lo = 0, hi = _rankCache.size() - 1;
    while (lo + 1 < hi) {
        size_t mid = (lo + hi) / 2;
        if (_rankCache[mid] < k) lo = mid; else hi = mid;
    }
    uint64_t count = _rankCache[lo];
    for (uint64_t i = lo * 512; i < _bitCount; ++i) {
        if (getBit(i)) { ++count; if (count == k) return i; }
    }
    return _bitCount;
}

int64_t SuccinctBitVector::child(uint64_t nodePos, int octant) const {
    uint64_t bitPos = nodePos * 8 + static_cast<uint64_t>(octant);
    if (bitPos >= _bitCount) return -1;
    if (!getBit(bitPos))     return -1;
    uint64_t idx = (bitPos > 0) ? rank1(bitPos - 1) : 0;
    return static_cast<int64_t>(idx);
}

void SuccinctBitVector::printBitString(size_t maxBits) const {
    size_t show = std::min(maxBits, _bitCount);
    std::cout << "  [SBV] First " << show << " bits: ";
    for (size_t i = 0; i < show; ++i) {
        if (i > 0 && i % 8 == 0) std::cout << ' ';
        std::cout << (getBit(i) ? '1' : '0');
    }
    if (_bitCount > maxBits) std::cout << " ...";
    std::cout << "\n";
}

void SuccinctBitVector::printMemoryComparison(size_t pointerBasedBytes) const {
    size_t sbvTotal = rawMemoryBytes() + rankStructBytes();

    std::cout << "\n--- FEATURE 6: Succinct Bit Vector — Memory Report ---\n";
    std::cout << std::string(56, '-') << "\n";
    std::cout << "  Octree nodes encoded      : " << (_bitCount / 8) << "\n";
    std::cout << "  LOUDS bit-string length   : " << _bitCount << " bits\n";
    std::cout << std::string(56, '-') << "\n";
    std::cout << "  Pointer-based storage     : " << pointerBasedBytes << " bytes\n";
    std::cout << "  Packed bit array          : " << rawMemoryBytes()  << " bytes\n";
    std::cout << "  Rank superblock cache     : " << rankStructBytes() << " bytes\n";
    std::cout << "  Total SBV memory          : " << sbvTotal          << " bytes\n";
    std::cout << std::string(56, '-') << "\n";

    if (pointerBasedBytes > 0) {
        double reduction = 100.0 * (1.0 - static_cast<double>(sbvTotal)
                                          / static_cast<double>(pointerBasedBytes));
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  Memory reduction          : " << reduction << "%\n";
        std::cout << "  Compression factor        : "
                  << (static_cast<double>(pointerBasedBytes) / static_cast<double>(sbvTotal))
                  << "x\n";
    }
    std::cout << std::string(56, '-') << "\n";
    std::cout << "  Insight: 8 child pointers (64 B/node) -> 8 bits (1 B/node).\n"
              << "  Rank/select stay O(1) via 512-bit superblock cache.\n";
    printBitString(64);
}

// SuccinctOctreeEncoder

SuccinctBitVector SuccinctOctreeEncoder::encode(const Octree& tree,
                                                 size_t& outPointerBasedBytes) {
    std::vector<std::array<bool, 8>> masks = tree.getChildMasks();
    outPointerBasedBytes = masks.size() * 8 * sizeof(void*);

    SuccinctBitVector sbv;
    sbv.encode(masks);
    return sbv;
}

//  DEMO RUNNER

void runAdvancedStructuresDemos(
    const std::vector<std::pair<int, std::string>>& lightCurves,
    const std::vector<Star>& stars,
    const Octree& octree)
{
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "   ADVANCED STRUCTURES DEMO\n";
    std::cout << std::string(60, '=') << "\n";

    // 1. Huffman
    {
        HuffmanCompressor hc;
        hc.compressAll(lightCurves);
    }

    // 2. B-Tree
    {
        BTreeStarIndex btree;
        std::cout << "\n  [B-Tree] Indexing " << stars.size() << " stars...\n";
        btree.buildIndex(stars);
        for (uint64_t qid : {1ULL, 5ULL, 10ULL, 999ULL})
            btree.printVsBSTComparison(qid);
    }

    // 3. Succinct Bit Vector
    {
        size_t ptrBytes = 0;
        SuccinctBitVector sbv = SuccinctOctreeEncoder::encode(octree, ptrBytes);
        sbv.printMemoryComparison(ptrBytes);
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "   Advanced Structures Demo Complete\n";
    std::cout << std::string(60, '=') << "\n";
}