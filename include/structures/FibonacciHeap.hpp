#ifndef FIBONACCIHEAP_HPP
#define FIBONACCIHEAP_HPP

#include <string>
#include <cmath>
#include <vector>
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>

/**
 * @brief Fibonacci Heap for O(1) amortized insert and O(log n) extract-min.
 * Used to prioritize real-time astronomical events by urgency score.
 */
struct AstroEvent {
    double priority;    // Lower = more urgent
    uint64_t starId;
    std::string description;

    bool operator>(const AstroEvent& other) const { return priority > other.priority; }
};

class FibonacciHeap {
private:
    struct Node {
        AstroEvent event;
        int degree = 0;
        bool marked = false;
        Node* parent = nullptr;
        Node* child = nullptr;
        Node* left = nullptr;
        Node* right = nullptr;

        explicit Node(AstroEvent e) : event(std::move(e)) {
            left = right = this;
        }
    };

    Node* minNode = nullptr;
    int nodeCount = 0;

    void link(Node* y, Node* x);
    void consolidate();
    void cut(Node* x, Node* y);
    void cascadingCut(Node* y);
    void addToRootList(Node* node);

public:
    FibonacciHeap() = default;
    ~FibonacciHeap();

    void insert(AstroEvent event);
    AstroEvent extractMin();
    bool isEmpty() const { return minNode == nullptr; }
    int size() const { return nodeCount; }

    // Peek at minimum without removing
    const AstroEvent& peek() const { return minNode->event; }
};

#endif