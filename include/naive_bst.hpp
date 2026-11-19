// naive_bst.hpp -- unbalanced binary search tree, the "before AVL" baseline
// used only by the benchmark to show what balancing buys.
#pragma once

#include <cstddef>

#include "avl_tree.hpp"  // reuses Node and TreeMetrics

class NaiveBST {
public:
    NaiveBST() noexcept = default;
    ~NaiveBST() { clear(); }
    NaiveBST(const NaiveBST&) = delete;
    NaiveBST& operator=(const NaiveBST&) = delete;

    void insert(int key, int val, heritage::TreeMetrics& m);
    bool find(int key, int& out, heritage::TreeMetrics& m) const;
    void clear() noexcept;
    int height() const;

private:
    Node* root_{nullptr};
};
