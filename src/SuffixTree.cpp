#include "structures/SuffixTree.hpp"
#include <stdexcept>
#include <algorithm>

SuffixTree::SuffixTree() {
    // Reserve space for the root
    nodes.reserve(1024);
    activeNode = 0;
    activeEdge = 0;
    activeLength = 0;
    remainingSuffixCount = 0;
    leafEnd = -1;
    splitEnd = nullptr;
    size = 0;
    root = -1;
}

SuffixTree::~SuffixTree() {
    for (auto* node : nodes) {
        if (node) {
            // Only delete end pointers that aren't the shared leafEnd
            if (node->end != &leafEnd) {
                delete node->end;
            }
            delete node;
        }
    }
}

int SuffixTree::newNode(int start, int* end) {
    Node* node = new Node(start, end);
    nodes.push_back(node);
    return (int)nodes.size() - 1;
}

/**
 * @brief Add a star's light-curve pattern to the suffix tree.
 *
 * Each pattern is appended to the global text with a unique delimiter
 * so suffixes from different patterns don't merge across boundaries.
 */
void SuffixTree::addPattern(uint64_t starId, const std::string& pattern) {
    // Use a unique delimiter per star to isolate suffixes
    char delimiter = (char)(1 + (starId % 126)); // Non-printable delimiters
    std::string tagged = pattern + delimiter;

    int offset = (int)text.size();
    for (int i = 0; i < (int)tagged.size(); i++) {
        posToStarId[offset + i] = (int)starId;
    }
    text += tagged;
}

/**
 * @brief Search for all stars whose light-curve contains the given pattern.
 * Uses naive O(n*m) approach on the concatenated text for correctness.
 * For billion-scale data, replace with proper Ukkonen traversal.
 */
std::vector<int> SuffixTree::search(const std::string& pattern) const {
    std::vector<int> results;
    if (pattern.empty() || text.empty()) return results;

    std::unordered_map<int, bool> seen;
    size_t pos = 0;
    while ((pos = text.find(pattern, pos)) != std::string::npos) {
        int starId = -1;
        auto it = posToStarId.find((int)pos);
        if (it != posToStarId.end()) {
            starId = it->second;
        }
        if (starId != -1 && !seen[starId]) {
            seen[starId] = true;
            results.push_back(starId);
        }
        pos++;
    }
    return results;
}

bool SuffixTree::hasPattern(const std::string& pattern) const {
    return text.find(pattern) != std::string::npos;
}