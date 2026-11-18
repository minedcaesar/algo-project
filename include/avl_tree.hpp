// avl_tree.hpp -- AVL tree (Adelson-Velsky & Landis, 1962) behind the course's
// "Project type 3: Tree structures" interface.
#pragma once

#include <cstddef>
#include <cstdint>

#include "metrics.hpp"

// Node layout required by the course interface, plus one field added by us:
// `balance`, the AVL balance indicator, defined as height(right) - height(left)
// and always in {-1, 0, +1} between operations. The 1962 paper stores exactly
// this three-valued indicator per vertex instead of a height.
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
    // and the shape of the tree does not change.
    void insert(int key, int val, TreeMetrics& m);
    // Returns true and writes the value to `out` if key is present.
    bool find(int key, int& out, TreeMetrics& m) const;
    // Writes up to `cap` keys in ascending order into out_buf.
    void inorder(int* out_buf, std::size_t cap) const;
    // Deletes every node; the tree is empty and reusable afterwards.
    void clear() noexcept;

    // --- Extensions (not part of the required interface) -------------------
    // Removes key if present; returns whether something was removed.
    // Deletion is NOT in the 1962 paper; see REPORT.md, "Legacy".
    bool erase(int key, TreeMetrics& m);
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    // Height in edges+1 (empty tree = 0, single node = 1). O(log n): follows
    // the balance indicators down one path.
    int height() const noexcept;
    // Read-only access for invariant checks in tests.
    const Node* root() const noexcept { return root_; }

private:
    Node* root_{nullptr};
    std::size_t size_{0};

    void replace_child(Node* parent, Node* old_child, Node* new_child) noexcept;
    Node* rotate_left(Node* x, TreeMetrics& m) noexcept;
    Node* rotate_right(Node* x, TreeMetrics& m) noexcept;
    Node* rebalance(Node* x, TreeMetrics& m) noexcept;
    static Node* clone(const Node* src, Node* parent);
};
