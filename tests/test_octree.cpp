#include <iostream>
#include <cassert>
#include "core/Star.hpp"
#include "structures/Octree.hpp"

int passed = 0, failed = 0;

#define TEST(name, expr) \
    if (expr) { std::cout << "  [PASS] " << name << std::endl; passed++; } \
    else       { std::cout << "  [FAIL] " << name << std::endl; failed++; }

int main() {
    std::cout << "=== Octree Unit Tests ===" << std::endl;

    Boundary world = { Vector3D(0,0,0), 1000.0 };
    Octree tree(world);

    // Create test stars
    Star s1(1, "Sol",    Vector3D(0,   0,   0));
    Star s2(2, "Sirius", Vector3D(8.6, 0,   0));
    Star s3(3, "Far",    Vector3D(900, 900, 900));
    Star s4(4, "Near",   Vector3D(5,   5,   5));

    TEST("Insert Sol",    tree.insert(&s1));
    TEST("Insert Sirius", tree.insert(&s2));
    TEST("Insert Far",    tree.insert(&s3));
    TEST("Insert Near",   tree.insert(&s4));

    // Out-of-bounds insert should fail
    Star sOut(99, "Outside", Vector3D(2000, 0, 0));
    TEST("Out-of-bounds rejected", !tree.insert(&sOut));

    // Range query near origin
    Boundary small = { Vector3D(0,0,0), 10.0 };
    std::vector<Star*> results;
    tree.queryRange(small, results);
    TEST("Query near origin finds >= 2 stars", results.size() >= 2);

    bool foundSol    = false, foundSirius = false;
    for (Star* s : results) {
        if (s->id == 1) foundSol    = true;
        if (s->id == 2) foundSirius = true;
    }
    TEST("Sol found in range query",    foundSol);
    TEST("Sirius found in range query", foundSirius);

    // Distant star should NOT appear in small query
    bool foundFar = false;
    for (Star* s : results) if (s->id == 3) foundFar = true;
    TEST("Far star NOT in small range", !foundFar);

    // Total tree size
    TEST("Tree holds 4 stars", tree.size() == 4);

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed." << std::endl;
    return failed == 0 ? 0 : 1;
}