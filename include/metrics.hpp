// metrics.hpp -- STAND-IN for the course reference header.
//
// The official metrics.hpp is distributed on Moodle and was not available when
// this repository was written. This file declares the subset of TreeMetrics
// used by the AVL implementation. If the official header differs, replace this
// file and adapt the few `m.<field>++` lines in src/avl_tree.cpp.
#pragma once

#include <cstdint>

struct TreeMetrics {
    std::uint64_t comparisons = 0;      // three-way key comparisons (one per node inspected)
    std::uint64_t rotations = 0;        // elementary rotations (a double rotation counts 2)
    std::uint64_t single_rotations = 0; // rebalancing events solved by one rotation
    std::uint64_t double_rotations = 0; // rebalancing events solved by two rotations
    std::uint64_t balance_updates = 0;  // balance-indicator writes while retracing
    std::uint64_t allocations = 0;      // nodes allocated with new
    std::uint64_t deallocations = 0;    // nodes released with delete (erase only)

    void reset() noexcept { *this = TreeMetrics{}; }
};
