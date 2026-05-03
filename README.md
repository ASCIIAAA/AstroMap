# AstroMap — Galactic Navigation Engine v1.0

A C++ engine for real-time stellar data querying, pattern analysis, and galactic navigation, powered by a suite of advanced data structures. Includes a 3D interactive web visualiser (Orrery) and a full-featured browser-based CLI.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Data Structures](#data-structures)
- [Modules](#modules)
- [Web Interfaces](#web-interfaces)
- [Building & Running](#building--running)
- [CLI Usage](#cli-usage)
- [Benchmarks](#benchmarks)
- [Advanced Structures](#advanced-structures)

---

## Overview

AstroMap loads a catalogue of 30 stars (from `stars.csv` and `lightcurves.txt`) and indexes them across six purpose-built data structures simultaneously. Each structure is optimised for a different type of query — spatial proximity, integer ID lookup, light-curve pattern matching, multi-dimensional filtering, priority-based event scheduling, and memory-efficient tree encoding.

The engine exposes two web interfaces: a 3D Orrery for interactive exploration and a terminal-style CLI for programmatic querying.

---

## Features

- **3D Orrery** — rotate, zoom, and click any star to inspect its spectral profile and light-curve signature
- **VEB ID Navigator** — jump to any star by ID with O(log log U) lookup
- **Light-Curve Pattern Search** — scan all stars for a brightness pattern (e.g. `HHH`, `MMM`, `HL`) using a suffix tree
- **Range Tree Filter** — filter stars by apparent magnitude and/or temperature with preset queries (Hot & Bright, Cool Stars, Sun-like)
- **Priority Event Queue** — live Fibonacci Heap event log showing astronomical alerts sorted by urgency
- **ASCII Star Map** — 2D projections of the stellar neighbourhood (XY and XZ planes)
- **Huffman Compression** — lossless encoding of light-curve strings (average 7.12× ratio)
- **B-Tree Indexing** — disk-efficient star lookups (1 page read vs ~5 BST node reads)
- **Succinct Bit Vector** — LOUDS-encoded Octree with 96.3% memory reduction
- **Performance Benchmarks** — full ops/sec and complexity validation at N=1000 and N=10000

---

## Project Structure

```
AstroMap/
├── data/
│   ├── lightcurves.txt          # Light-curve strings per star (H/M/L patterns)
│   └── stars.csv                # Star catalogue (name, magnitude, temperature, position)
├── include/
│   └── core/
│       ├── AsciiStarMap.hpp
│       ├── Star.hpp
│       └── Vector3D.hpp
├── engine/
│   └── GalacticEngine.hpp
├── structures/
│   ├── advanced_structures.hpp
│   ├── FibonacciHeap.hpp
│   ├── Octree.hpp
│   ├── RangeTree.hpp
│   ├── SuffixTree.hpp
│   ├── VEBTree.hpp
│   └── benchmark.hpp
├── src/
│   ├── advanced_structures.cpp
│   ├── AsciiStarMap.cpp
│   ├── benchmark.cpp
│   ├── FibonacciHeap.cpp
│   ├── GalacticEngine.cpp
│   ├── Octree.cpp
│   ├── RangeTree.cpp
│   ├── SuffixTree.cpp
│   └── VEBTree.cpp
├── tests/
│   ├── test_advanced.cpp
│   ├── test_benchmark.cpp
│   └── test_veb.cpp
├── web/
│   ├── cli.html                 # Browser-based CLI interface
│   ├── engine.js                # JS engine binding
│   ├── index.html               # 3D Orrery interface
│   ├── stars.json               # Exported star data for web
│   └── styles.css
├── astromap.exe
├── CMakeLists.txt
├── main.cpp
└── stars.csv
```

---

## Data Structures

| Structure | Purpose | Time Complexity |
|---|---|---|
| **Octree** | 3D spatial range queries | O(log n) insert · O(k + log n) query |
| **Van Emde Boas Tree** | Integer ID lookup and successor | O(log log U) all ops |
| **Suffix Tree** | Light-curve pattern matching | O(n) build (Ukkonen) · O(m) search |
| **Range Tree** | 2D magnitude + temperature filter | O(log² n) insert · O(log² n + k) query |
| **Fibonacci Heap** | Priority event queue | O(1) amortised insert · O(log n) extract-min |
| **B-Tree** | Disk-efficient ID indexing | O(log n) · 1 page read per lookup |
| **Huffman Coding** | Light-curve string compression | O(n log n) build · lossless |
| **Succinct Bit Vector** | LOUDS-encoded Octree structure | O(1) rank/select via superblock cache |

---

## Modules

### Module 1 — Octree Spatial Query
Answers the question *"which stars are within X light-years of a given point?"* by recursively subdividing 3D space into octants and pruning regions that cannot contain a match. Also supports nearest-neighbour queries.

**Example:** Stars within 8 ly of (10, 0, 0) → 10 results including Sirius, Proxima Centauri, and Procyon.

### Module 2 — Van Emde Boas ID Lookup
Maps integer star IDs to star objects in O(log log U) time by splitting the ID universe into √U-sized clusters, recursively. Supports successor queries to iterate IDs in sorted order.

**Example:** `ID 5 → Arcturus`, `ID 99 → NOT FOUND`, sorted IDs: `1 2 3 4 5 6 7 8 9 10`

### Module 3 — Suffix Tree Pattern Matching
Each star's photometric behaviour is encoded as a string of `H` (high), `M` (mid), and `L` (low) brightness symbols. All strings are indexed in a single suffix tree using Ukkonen's linear-time algorithm. Pattern queries run in O(m) time, independent of catalogue size.

**Example patterns:**
- `HL` → Canopus, Betelgeuse (oscillating brightness)
- `MMM` → Sol, Sirius, Alpha Centauri A, Vega, Rigel (stable)
- `HHH` → Vega, Rigel (consistently high output)

### Module 4 — Range Tree Multi-Dimensional Filter
A BST on magnitude where each internal node stores a secondary BST on temperature. Supports orthogonal 2D range queries with fractional cascading for efficient lookups.

**Preset queries:**
- **Hot & Bright** — mag ∈ [−2, 0.5], temp ∈ [7000, 15000] K → 5 stars
- **Cool Stars** — temp < 5000 K → 10 stars
- **Visually Brightest** — mag < 0 → Sirius, Canopus, Arcturus
- **Sun-like** — mag ∈ [0, 6], temp ∈ [5000, 6500] K → Epsilon Eridani, Alpha Centauri A, Sol

### Module 5 — Fibonacci Heap Event Queue
Astronomical alerts are inserted as (priority, star, message) triples. The Fibonacci Heap's O(1) amortised insert makes queuing new events effectively free; extract-min dispatches the most urgent alert in O(log n).

**Event priorities (lowest = most urgent):**
1. 0.5 — Canopus X-ray burst (CRITICAL)
2. 1.0 — Betelgeuse supernova precursor (URGENT)
3. 1.7 — Vega debris disk asymmetry (ALERT)
4. 2.2 — Sirius companion mass transfer (WARNING)
5. 3.5 — Rigel luminosity spike +15% (ALERT)
6. 4.8 — Arcturus spectral shift nominal (INFO)

### Module 6 — ASCII Star Map
Projects 3D star positions onto 2D planes for terminal display. Two projections are rendered: XY (top-down galactic view) and XZ (edge-on view at 1.5× zoom). Glyph brightness encodes apparent magnitude: `*` very bright · `o` bright · `+` medium · `.` dim.

---

## Web Interfaces

### 3D Orrery (`index.html`)
An interactive three.js-powered star viewer rendered in the browser.

- **Drag** to rotate · **Scroll** to zoom · **Click** any star to focus
- Projection modes: XY · XZ · YZ
- **VEB ID Navigator** — enter any ID (1–30) and jump directly to that star
- **Light-Curve Pattern Search** — type a pattern (e.g. `HHH`) and scan all stars
- **Range Tree Filter** — dual sliders for magnitude and temperature; preset buttons for common queries
- **Priority Event Queue** — live Fibonacci Heap log at the bottom of the screen
- **Spectral Profile** and **Light-Curve Signature** panels populate on star selection
- **Octree Density Heatmap** — minimap in the bottom right corner
- Performance dashboard shows real-time stats: Stars Total, vEB Universe size, complexity labels, Target FPS

### CLI Interface (`cli.html`)
A full terminal-style interface with tab-completion and command history.

**Navigation commands:**
```
query --all                  List all 30 stars
query --name Sirius          Look up a star by name
query --spectral B           Filter by spectral type
query --mag-max 1.0          Stars brighter than magnitude 1.0
query --mag-min 8            Stars dimmer than magnitude 8
query --temp-min 20000       Hot stars above 20000 K
query --dist-max 15          Stars within 15 light-years
query --sort mag             Sort results by magnitude
```

**Tree and curve commands:**
```
tree --show octree           Visualise the Octree structure
tree --show range            Visualise the Range Tree
lc --star Sirius             Plot Sirius's light-curve
lc --star Betelgeuse
veb --jump 9                 VEB successor jump to ID 9
pattern --find HHH           Suffix tree search for pattern
pattern --find HL
```

---

## Building & Running

### Prerequisites
- C++17 or later
- CMake 3.15+

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run engine (CLI output)

```bash
./astromap
```

### Run web interfaces

Open `web/index.html` or `web/cli.html` directly in a browser. No server required — star data is pre-exported to `web/stars.json`.

---

## Benchmarks

All structures benchmarked at N=1,000 and N=10,000 with ops/sec and complexity validation.

| Structure | Operation | N=1000 | N=10000 | Complexity |
|---|---|---|---|---|
| Octree | Insert | 1.34 M/s | 1.85 M/s | O(log n) |
| Octree | QueryRange | 17.07 M/s | 61.87 M/s | O(k + log n) |
| VEB Tree | Insert | 532 K/s | 419 K/s | O(log log U) |
| VEB Tree | Contains | 1.22 M/s | 1.53 M/s | O(log log U) |
| Suffix Tree | Search | 8.80 K/s | 1.61 K/s | O(m) |
| Fibonacci Heap | Insert | 347 K/s | 364 K/s | O(1) amortised |
| Fibonacci Heap | ExtractMin | 284 K/s | 37 K/s | O(log n) amortised |
| Range Tree | Query2D | 14.83 M/s | 8.35 M/s | O(log² n + k) |
| Range Tree | QueryMag | 17.29 M/s | 13.23 M/s | O(log n + k) |

Notable: Octree range query throughput *increases* with N — larger trees prune more aggressively, which is the expected behaviour of a well-implemented spatial index.

---

## Advanced Structures

### Huffman Light-Curve Compression

Light-curve strings are compressed per-star using a frequency-based Huffman code. A string of all `M`s (e.g. Sol) compresses from 128 bits to 16 bits (8× ratio) because the entire alphabet reduces to a single 1-bit code. More varied patterns compress less aggressively.

- Average compression ratio across 10 stars: **7.12×**
- All round-trip checks: **PASS**

### B-Tree vs BST Disk Comparison

Stars are indexed in a B-Tree with page size 4096 B and degree T=28 (55 keys/node). All 30 stars fit in a single page (height = 1), meaning any lookup costs exactly **1 disk read**, versus ~5 pointer chases in a conventional BST — an 80% reduction in I/O.

### Succinct Bit Vector (LOUDS Encoding)

The Octree's 169-node structure is encoded as a LOUDS bitstring (1352 bits). Rank and select operations remain O(1) via a 512-bit superblock cache. This reduces tree storage from 5408 bytes (pointer-based) to 201 bytes total.

- **Memory reduction: 96.3%**
- **Compression factor: 26.9×**