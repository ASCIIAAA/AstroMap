#include "structures/advanced_structures.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// Minimal test harness
static int g_pass = 0, g_fail = 0;

#define EXPECT(cond)                                                          \
    do {                                                                      \
        if (cond) { ++g_pass; std::cout << "  PASS\n"; }                    \
        else      { ++g_fail; std::cout << "  FAIL  (line " << __LINE__ << ")\n"; } \
    } while(0)

#define EXPECT_EQ(a, b) EXPECT((a) == (b))

static void heading(const char* s) {
    std::cout << "\n[" << s << "]\n";
}
static void test(const char* name) {
    std::cout << "  " << name << " ... ";
}

//  Huffman tests
static void testHuffman() {
    heading("Huffman Compression");
    HuffmanCompressor hc;

    test("Round-trip: MMMMMMMMMM");
    { HuffmanResult r = hc.compress("MMMMMMMMMM");
      EXPECT(hc.decompress(r) == std::string("MMMMMMMMMM")); }

    test("Round-trip: HMMMHMMHMMMHMMHM");
    { std::string s = "HMMMHMMHMMMHMMHM";
      EXPECT(hc.decompress(hc.compress(s)) == s); }

    test("Round-trip: HLHLHLHLHLHLHLHL");
    { std::string s = "HLHLHLHLHLHLHLHL";
      EXPECT(hc.decompress(hc.compress(s)) == s); }

    test("Single-char round-trip: LLLLLLLL");
    { std::string s = "LLLLLLLL";
      EXPECT(hc.decompress(hc.compress(s)) == s); }

    test("Compression ratio > 1 for biased string");
    { std::string biased(100, 'M');
      biased += std::string(5, 'H') + std::string(3, 'L');
      EXPECT(hc.compress(biased).compressionRatio() > 1.0); }

    test("Compressed bits < raw bits for mixed string");
    { std::string s = "MHLMHLMHLMHLMHLM";
      EXPECT(hc.compress(s).bitLength < s.size() * 8); }

    test("Empty string");
    { EXPECT_EQ(hc.compress("").bitLength, static_cast<size_t>(0)); }

    test("All 10 light curves round-trip correctly");
    {
        const std::vector<std::pair<int, std::string>> curves = {
            {1, "MMMMMMMMMMMMMMMM"}, {2, "HMMMHMMHMMMHMMHM"},
            {3, "MHLMHLMHLMHLMHLM"}, {4, "MMMMMMMMMMMMMMMM"},
            {5, "LMLMLMLMLMLMLMLM"}, {6, "HHHMMMLLLMMMHHHM"},
            {7, "LLLMMMHHHMMMLLLM"}, {8, "MMHMMHMMHMMHMMHM"},
            {9, "HLHLHLHLHLHLHLHL"},{10, "MHMHMHMHMHMHMHMH"},
        };
        bool ok = true;
        for (const auto& kv : curves) {
            if (hc.decompress(hc.compress(kv.second)) != kv.second) { ok = false; break; }
        }
        EXPECT(ok);
    }
}

//  B-Tree tests
static std::vector<Star> makeSyntheticStars(int n) {
    std::vector<Star> stars;
    stars.reserve(n);
    for (int i = 1; i <= n; ++i) {
        Star s;
        s.id          = static_cast<uint64_t>(i);
        s.name        = "Star_" + std::to_string(i);
        s.magnitude   = static_cast<float>(i) * 0.3f;
        s.temperature = 3000.0f + static_cast<float>(i) * 200.0f;
        s.position    = Vector3D(i, i, i);
        stars.push_back(s);
    }
    return stars;
}

static void testBTree() {
    heading("B-Tree Disk Index");
    auto stars = makeSyntheticStars(20);
    BTreeStarIndex idx;
    idx.buildIndex(stars);

    test("All 20 inserted stars are found");
    {
        bool ok = true;
        for (int i = 1; i <= 20; ++i) {
            int reads = 0;
            if (!idx.search(static_cast<uint64_t>(i), reads)) { ok = false; break; }
        }
        EXPECT(ok);
    }

    test("Non-existent ID returns nullopt");
    { int r = 0; EXPECT(!idx.search(999ULL, r)); }

    test("Disk reads <= tree height");
    { int r = 0; idx.search(10ULL, r); EXPECT(r <= idx.height()); }

    test("Retrieved record has correct magnitude");
    { int r = 0;
      auto rec = idx.search(5ULL, r);
      EXPECT(rec.has_value() && std::fabs(rec->magnitude - 5 * 0.3f) < 0.001f); }

    test("T degree >= 2");
    { EXPECT(idx.tDegree() >= 2); }

    test("500-star index: spot checks all found within height");
    {
        auto big = makeSyntheticStars(500);
        BTreeStarIndex bigIdx;
        bigIdx.buildIndex(big);
        bool ok = true;
        for (int i : {1, 50, 100, 250, 499, 500}) {
            int r = 0;
            auto rec = bigIdx.search(static_cast<uint64_t>(i), r);
            if (!rec || r > bigIdx.height()) { ok = false; break; }
        }
        EXPECT(ok);
    }
}

//  Succinct Bit Vector tests
static void testSuccinctBitVector() {
    heading("Succinct Bit Vector");

    // 4 mock Octree nodes
    std::vector<std::array<bool, 8>> masks(4);
    masks[0] = {true, true, true, true, false, false, false, false};
    masks[1] = {false, false, false, false, true, true, false, false};
    masks[2] = {false, false, false, false, false, false, false, false};
    masks[3] = {false, false, false, false, false, false, false, true};

    SuccinctBitVector sbv;
    sbv.encode(masks);

    test("Bit count == nodes * 8");
    { EXPECT_EQ(sbv.bitCount(), static_cast<size_t>(32)); }

    test("Raw memory < pointer-based (256 bytes)");
    { EXPECT(sbv.rawMemoryBytes() < static_cast<size_t>(256)); }

    test("rank1(7) == 4  (first 8 bits = 11110000)");
    { EXPECT_EQ(sbv.rank1(7), static_cast<uint64_t>(4)); }

    test("rank1(15) == 6  (first 16 bits have 6 ones)");
    { EXPECT_EQ(sbv.rank1(15), static_cast<uint64_t>(6)); }

    test("select1(1) == 0  (first 1-bit at position 0)");
    { EXPECT_EQ(sbv.select1(1), static_cast<uint64_t>(0)); }

    test("child(0, 0) >= 0  (exists)");
    { EXPECT(sbv.child(0, 0) >= 0); }

    test("child(0, 4) == -1  (absent)");
    { EXPECT_EQ(sbv.child(0, 4), static_cast<int64_t>(-1)); }

    test("child(2, 0) == -1  (all-empty node)");
    { EXPECT_EQ(sbv.child(2, 0), static_cast<int64_t>(-1)); }

    test("Total SBV bytes < naive 8-pointer layout");
    {
        size_t naive = masks.size() * 8 * sizeof(void*);
        size_t total = sbv.rawMemoryBytes() + sbv.rankStructBytes();
        EXPECT(total < naive);
    }
}

//  main
int main() {
    std::cout << "============================================================\n";
    std::cout << "   AstroMap — Advanced Structures Test Suite\n";
    std::cout << "============================================================\n";

    testHuffman();
    testBTree();
    testSuccinctBitVector();

    std::cout << "\n============================================================\n";
    std::cout << "  Results:  " << g_pass << " passed,  " << g_fail << " failed\n";
    std::cout << "============================================================\n";
    return (g_fail == 0) ? 0 : 1;
}