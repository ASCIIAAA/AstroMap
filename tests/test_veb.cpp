#include <iostream>
#include <cassert>
#include "structures/vEBTree.hpp"

int passed = 0, failed = 0;

#define TEST(name, expr) \
    if (expr) { std::cout << "  [PASS] " << name << std::endl; passed++; } \
    else       { std::cout << "  [FAIL] " << name << std::endl; failed++; }

int main() {
    std::cout << "=== Van Emde Boas Tree Unit Tests ===" << std::endl;

    // Universe size: 2^10 = 1024
    vEBTree veb(1024);

    TEST("Empty tree isEmpty",       veb.isEmpty());
    TEST("Contains nothing yet",     !veb.contains(5));

    veb.insert(1);
    veb.insert(5);
    veb.insert(7);
    veb.insert(10);
    veb.insert(42);
    veb.insert(100);
    veb.insert(999);

    TEST("Not empty after inserts",  !veb.isEmpty());
    TEST("Contains 1",               veb.contains(1));
    TEST("Contains 5",               veb.contains(5));
    TEST("Contains 42",              veb.contains(42));
    TEST("Contains 999",             veb.contains(999));
    TEST("Does not contain 2",       !veb.contains(2));
    TEST("Does not contain 50",      !veb.contains(50));

    TEST("Minimum is 1",             veb.minimum() == 1);
    TEST("Maximum is 999",           veb.maximum() == 999);

    TEST("Successor of 1 is 5",      veb.successor(1) == 5);
    TEST("Successor of 5 is 7",      veb.successor(5) == 7);
    TEST("Successor of 42 is 100",   veb.successor(42) == 100);
    TEST("Predecessor of 10 is 7",   veb.predecessor(10) == 7);
    TEST("Predecessor of 5 is 1",    veb.predecessor(5) == 1);

    // Remove test
    veb.remove(5);
    TEST("After remove(5): not contains 5",     !veb.contains(5));
    TEST("After remove(5): successor(1) is 7",  veb.successor(1) == 7);
    TEST("Predecessor of 7 is now 1",           veb.predecessor(7) == 1);

    // Duplicate insert (idempotent)
    veb.insert(42);
    TEST("Re-insert 42: still contains 42",     veb.contains(42));

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed." << std::endl;
    return failed == 0 ? 0 : 1;
}