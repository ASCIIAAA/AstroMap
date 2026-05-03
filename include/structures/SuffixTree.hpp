#ifndef SUFFIXTREE_HPP
#define SUFFIXTREE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

/**
 * @brief Suffix Tree for O(m) pattern matching on stellar light-curve strings.
 *
 * Uses Ukkonen's algorithm for O(n) construction. Each leaf stores the
 * star ID whose light-curve string was inserted, enabling fast lookup of
 * which stars share a given brightness pattern.
 */
class SuffixTree {
private:
    struct Node {
        std::unordered_map<char, int> children; // char -> child node index
        int suffixLink = 0;
        int start;
        int* end;
        int suffixIndex = -1; // Which suffix this leaf represents
        int starId = -1;      // Which star's pattern

        Node(int start, int* end) : start(start), end(end) {}
        int edgeLength() const { return *end - start + 1; }
    };

    std::vector<Node*> nodes;
    std::string text;      // Concatenated patterns (with delimiters)
    int activeNode;
    int activeEdge;
    int activeLength;
    int remainingSuffixCount;
    int leafEnd;
    int* splitEnd;
    int size;
    int root;

    // Maps position in text to star ID
    std::unordered_map<int, int> posToStarId;

    int newNode(int start, int* end);
    void extendSuffixTree(int pos);
    void setSuffixIndexDFS(int n, int labelHeight);
    void searchDFS(int node, const std::string& pattern, int depth,
                   std::vector<int>& results) const;

public:
    SuffixTree();
    ~SuffixTree();

    // Build tree from a single star's light-curve pattern
    void addPattern(uint64_t starId, const std::string& pattern);

    // Find all star IDs whose light-curve contains the given pattern
    std::vector<int> search(const std::string& pattern) const;

    bool hasPattern(const std::string& pattern) const;
};

#endif