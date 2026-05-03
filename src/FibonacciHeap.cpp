#include "structures/FibonacciHeap.hpp"
#include <stdexcept>
#include <cmath>
#include <vector>

FibonacciHeap::~FibonacciHeap() {
    // Iterative cleanup of root list and all children
    std::vector<Node*> toDelete;
    if (!minNode) return;

    std::vector<Node*> stack;
    Node* curr = minNode;
    do {
        stack.push_back(curr);
        curr = curr->right;
    } while (curr != minNode);

    while (!stack.empty()) {
        Node* n = stack.back(); stack.pop_back();
        if (n->child) {
            Node* c = n->child;
            do {
                stack.push_back(c);
                c = c->right;
            } while (c != n->child);
        }
        delete n;
    }
}

void FibonacciHeap::addToRootList(Node* node) {
    node->parent = nullptr;
    if (!minNode) {
        minNode = node;
        node->left = node->right = node;
    } else {
        // Insert before minNode in circular doubly-linked list
        node->right = minNode;
        node->left = minNode->left;
        minNode->left->right = node;
        minNode->left = node;
        if (node->event.priority < minNode->event.priority) {
            minNode = node;
        }
    }
}

void FibonacciHeap::insert(AstroEvent event) {
    Node* node = new Node(std::move(event));
    addToRootList(node);
    nodeCount++;
}

void FibonacciHeap::link(Node* y, Node* x) {
    // Remove y from root list
    y->left->right = y->right;
    y->right->left = y->left;

    // Make y a child of x
    y->parent = x;
    if (!x->child) {
        x->child = y;
        y->left = y->right = y;
    } else {
        y->right = x->child;
        y->left = x->child->left;
        x->child->left->right = y;
        x->child->left = y;
    }
    x->degree++;
    y->marked = false;
}

void FibonacciHeap::consolidate() {
    // Capture the count BEFORE extraction loop begins, or use a safe upper bound:
    int maxDegree = static_cast<int>(std::log2(
        static_cast<double>(nodeCount) + 1.0)) + 2;

    maxDegree = static_cast<int>(1.5 * std::log2(
        static_cast<double>(nodeCount) + 2.0)) + 2;

    std::vector<Node*> degreeTable(maxDegree + 1, nullptr);

    // Collect all root nodes
    std::vector<Node*> roots;
    Node* curr = minNode;
    do {
        roots.push_back(curr);
        curr = curr->right;
    } while (curr != minNode);

    for (Node* w : roots) {
        Node* x = w;
        int d = x->degree;
        while (d <= maxDegree && degreeTable[d] != nullptr) {
            Node* y = degreeTable[d];
            if (x->event.priority > y->event.priority) std::swap(x, y);
            link(y, x);
            degreeTable[d] = nullptr;
            d++;
        }
        if (d <= maxDegree) degreeTable[d] = x;
    }

    // Rebuild root list from degree table
    minNode = nullptr;
    for (Node* node : degreeTable) {
        if (!node) continue;
        node->left = node->right = node;
        if (!minNode) {
            minNode = node;
        } else {
            node->right = minNode;
            node->left = minNode->left;
            minNode->left->right = node;
            minNode->left = node;
            if (node->event.priority < minNode->event.priority) {
                minNode = node;
            }
        }
    }
}

AstroEvent FibonacciHeap::extractMin() {
    if (!minNode) throw std::underflow_error("Heap is empty");

    Node* z = minNode;
    // Add all children to root list
    if (z->child) {
        std::vector<Node*> children;
        Node* c = z->child;
        do {
            children.push_back(c);
            c = c->right;
        } while (c != z->child);
        for (Node* child : children) {
            addToRootList(child);
            child->parent = nullptr;
        }
    }

    // Remove z from root list
    z->left->right = z->right;
    z->right->left = z->left;

    if (z == z->right) {
        minNode = nullptr;
    } else {
        minNode = z->right;
        consolidate();
    }

    nodeCount--;
    AstroEvent result = std::move(z->event);
    delete z;
    return result;
}