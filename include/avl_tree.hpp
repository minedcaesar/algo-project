// avl_tree.hpp -- AVL tree (Adelson-Velsky & Landis, 1962) behind the course's
// "Project type 3: Tree structures" interface.
#pragma once

#include <cstddef>
#include <cstdint>

#include "metrics.hpp"  // official course header (namespace heritage)

// Counters the course's heritage::TreeMetrics has no field for. Kept per tree
// (since construction or the last reset_stats()); a copy starts from zero.
struct AvlStats {
    std::uint64_t single_rotations{0}; // rebalancings solved by one rotation
    std::uint64_t double_rotations{0}; // rebalancings solved by two rotations
    std::uint64_t balance_updates{0};  // balance-indicator writes while retracing
    std::uint64_t node_frees{0};       // nodes deleted by erase()
};

// Node layout required by the course interface, plus one field added by us:
// `balance`, the AVL balance indicator, defined as height(right) - height(left)
// and always in {-1, 0, +1} between operations. The 1962 paper stores the same
// information as two bits per cell ("1 0" left higher, "0 1" right higher,
// "0 0" equal) instead of a height.
struct Node {
    int key;
    int value;
    Node* left{nullptr};
    Node* right{nullptr};
    Node* parent{nullptr};
    std::int8_t balance{0};

    Node(int k, int v, Node* p) noexcept : key(k), value(v), parent(p) {}
};

class SearchTree {
public:
    SearchTree() noexcept = default;
    ~SearchTree();

    // Rule of five: deep copy, pointer-stealing move.
    SearchTree(const SearchTree& other);
    SearchTree& operator=(const SearchTree& other);
    SearchTree(SearchTree&& other) noexcept;
    SearchTree& operator=(SearchTree&& other) noexcept;

    // --- Required interface -------------------------------------------------
    // Inserts (key, val). If key is already present its value is overwritten
    // and the shape of the tree does not change. (The paper leaves the tree
    // untouched on a duplicate; see REPORT.md section 2.4.)
    void insert(int key, int val, heritage::TreeMetrics& m);
    // Returns true and writes the value to `out` if key is present.
    bool find(int key, int& out, heritage::TreeMetrics& m) const;
    // Writes up to `cap` keys in ascending order into out_buf.
    void inorder(int* out_buf, std::size_t cap) const;
    // Deletes every node; the tree is empty and reusable afterwards.
    void clear() noexcept;

    // --- Extensions (not part of the required interface) -------------------
    // Removes key if present; returns whether something was removed.
    // Deletion is NOT in the 1962 paper; see REPORT.md, "Legacy".
    bool erase(int key, heritage::TreeMetrics& m);
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    // Height in edges+1 (empty tree = 0, single node = 1). O(log n): follows
    // the balance indicators down one path.
    int height() const noexcept;
    // Read-only access for invariant checks in tests.
    const Node* root() const noexcept { return root_; }
    // AVL-specific counters (see AvlStats).
    const AvlStats& stats() const noexcept { return stats_; }
    void reset_stats() noexcept { stats_ = AvlStats{}; }

private:
    Node* root_{nullptr};
    std::size_t size_{0};
    AvlStats stats_{};

    void replace_child(Node* parent, Node* old_child, Node* new_child) noexcept;
    Node* rotate_left(Node* x, heritage::TreeMetrics& m) noexcept;
    Node* rotate_right(Node* x, heritage::TreeMetrics& m) noexcept;
    Node* rebalance(Node* x, heritage::TreeMetrics& m) noexcept;
    static Node* clone(const Node* src, Node* parent);
};
