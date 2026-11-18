// avl_tree.cpp -- AVL tree implementation.
//
// Conventions
//   balance(x) = height(x->right) - height(x->left)  ("+" = right-heavy).
//   All operations are iterative except copy-construction (whose recursion
//   depth is bounded by the AVL height, < 1.45 log2 n).
#include "avl_tree.hpp"

#include <algorithm>
#include <utility>

// ---------------------------------------------------------------------------
// Lifetime
// ---------------------------------------------------------------------------
SearchTree::~SearchTree() { clear(); }

Node* SearchTree::clone(const Node* src, Node* parent) {
    if (src == nullptr) return nullptr;
    Node* n = new Node(src->key, src->value, parent);
    n->balance = src->balance;
    try {
        n->left = clone(src->left, n);
        n->right = clone(src->right, n);
    } catch (...) {
        // Release the partially built subtree so a failed copy cannot leak.
        n->parent = nullptr;  // so clear() stops at n
        SearchTree tmp;
        tmp.root_ = n;
        throw;  // tmp's destructor frees n and whatever was attached to it
    }
    return n;
}

SearchTree::SearchTree(const SearchTree& other)
    : root_(clone(other.root_, nullptr)), size_(other.size_) {}

SearchTree& SearchTree::operator=(const SearchTree& other) {
    if (this != &other) {
        SearchTree tmp(other);  // copy-and-swap: strong exception guarantee
        std::swap(root_, tmp.root_);
        std::swap(size_, tmp.size_);
    }
    return *this;
}

SearchTree::SearchTree(SearchTree&& other) noexcept
    : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0)) {}

SearchTree& SearchTree::operator=(SearchTree&& other) noexcept {
    if (this != &other) {
        clear();
        root_ = std::exchange(other.root_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

// Post-order deletion without recursion or an explicit stack: walk down to a
// leaf, detach it from its parent, delete it, and continue from the parent.
void SearchTree::clear() noexcept {
    Node* n = root_;
    while (n != nullptr) {
        if (n->left != nullptr) {
            n = n->left;
        } else if (n->right != nullptr) {
            n = n->right;
        } else {
            Node* p = n->parent;
            if (p != nullptr) {
                if (p->left == n) p->left = nullptr;
                else p->right = nullptr;
            }
            delete n;
            n = p;
        }
    }
    root_ = nullptr;
    size_ = 0;
}

// ---------------------------------------------------------------------------
// Rotations
// ---------------------------------------------------------------------------
void SearchTree::replace_child(Node* parent, Node* old_child, Node* new_child) noexcept {
    if (parent == nullptr) root_ = new_child;
    else if (parent->left == old_child) parent->left = new_child;
    else parent->right = new_child;
    if (new_child != nullptr) new_child->parent = parent;
}

/*      x                  y
 *     / \                / \
 *    a   y       ==>    x   c
 *       / \            / \
 *      b   c          a   b
 */
// Balance update for arbitrary (not only insertion-time) balances:
//   x' = x - 1 - max(y, 0)
//   y' = y - 1 + min(x', 0)
Node* SearchTree::rotate_left(Node* x, TreeMetrics& m) noexcept {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != nullptr) y->left->parent = x;
    replace_child(x->parent, x, y);
    y->left = x;
    x->parent = y;

    const int xb = x->balance - 1 - std::max<int>(y->balance, 0);
    const int yb = y->balance - 1 + std::min(xb, 0);
    x->balance = static_cast<std::int8_t>(xb);
    y->balance = static_cast<std::int8_t>(yb);
    ++m.rotations;
    return y;
}

// Mirror image of rotate_left:
//   x' = x + 1 - min(y, 0)
//   y' = y + 1 + max(x', 0)
Node* SearchTree::rotate_right(Node* x, TreeMetrics& m) noexcept {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != nullptr) y->right->parent = x;
    replace_child(x->parent, x, y);
    y->right = x;
    x->parent = y;

    const int xb = x->balance + 1 - std::min<int>(y->balance, 0);
    const int yb = y->balance + 1 + std::max(xb, 0);
    x->balance = static_cast<std::int8_t>(xb);
    y->balance = static_cast<std::int8_t>(yb);
    ++m.rotations;
    return y;
}

// x has balance +-2. Restores the AVL property at x with a single or double
// rotation and returns the new root of the subtree. Child balance 0 can only
// occur during deletion; a single rotation is correct in that case.
Node* SearchTree::rebalance(Node* x, TreeMetrics& m) noexcept {
    if (x->balance < 0) {           // left-heavy
        if (x->left->balance > 0) { // left-right case
            rotate_left(x->left, m);
            ++m.double_rotations;
        } else {
            ++m.single_rotations;   // left-left case
        }
        return rotate_right(x, m);
    }
    if (x->right->balance < 0) {    // right-left case
        rotate_right(x->right, m);
        ++m.double_rotations;
    } else {
        ++m.single_rotations;       // right-right case
    }
    return rotate_left(x, m);
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
bool SearchTree::find(int key, int& out, TreeMetrics& m) const {
    const Node* n = root_;
    while (n != nullptr) {
        ++m.comparisons;
        if (key < n->key) n = n->left;
        else if (n->key < key) n = n->right;
        else {
            out = n->value;
            return true;
        }
    }
    return false;
}

int SearchTree::height() const noexcept {
    int h = 0;
    for (const Node* n = root_; n != nullptr; n = (n->balance > 0 ? n->right : n->left)) ++h;
    return h;
}

void SearchTree::inorder(int* out_buf, std::size_t cap) const {
    if (out_buf == nullptr || cap == 0 || root_ == nullptr) return;
    // Threaded-style walk using parent pointers: O(n) time, O(1) extra space.
    const Node* n = root_;
    while (n->left != nullptr) n = n->left;
    std::size_t i = 0;
    while (n != nullptr && i < cap) {
        out_buf[i++] = n->key;
        if (n->right != nullptr) {
            n = n->right;
            while (n->left != nullptr) n = n->left;
        } else {
            const Node* p = n->parent;
            while (p != nullptr && n == p->right) {
                n = p;
                p = p->parent;
            }
            n = p;
        }
    }
}

// ---------------------------------------------------------------------------
// Insertion (the operation described in the 1962 paper)
// ---------------------------------------------------------------------------
void SearchTree::insert(int key, int val, TreeMetrics& m) {
    if (root_ == nullptr) {
        root_ = new Node(key, val, nullptr);
        ++m.allocations;
        size_ = 1;
        return;
    }

    // 1. Ordinary BST descent.
    Node* p = root_;
    Node* child = nullptr;
    for (;;) {
        ++m.comparisons;
        if (key < p->key) {
            if (p->left == nullptr) {
                child = p->left = new Node(key, val, p);
                break;
            }
            p = p->left;
        } else if (p->key < key) {
            if (p->right == nullptr) {
                child = p->right = new Node(key, val, p);
                break;
            }
            p = p->right;
        } else {
            p->value = val;  // key already present: update in place
            return;
        }
    }
    ++m.allocations;
    ++size_;

    // 2. Retrace towards the root. `child`'s subtree just grew by one level.
    while (p != nullptr) {
        p->balance = static_cast<std::int8_t>(p->balance + (child == p->left ? -1 : +1));
        ++m.balance_updates;
        if (p->balance == 0) return;  // p's height unchanged: done
        if (p->balance == 1 || p->balance == -1) {
            child = p;                // p grew: propagate upwards
            p = p->parent;
            continue;
        }
        // |balance| == 2: one (single or double) rotation restores the
        // subtree's pre-insertion height, so nothing above changes.
        rebalance(p, m);
        return;
    }
}

// ---------------------------------------------------------------------------
// Deletion (extension; due to later authors, not the 1962 paper)
// ---------------------------------------------------------------------------
bool SearchTree::erase(int key, TreeMetrics& m) {
    Node* z = root_;
    while (z != nullptr) {
        ++m.comparisons;
        if (key < z->key) z = z->left;
        else if (z->key < key) z = z->right;
        else break;
    }
    if (z == nullptr) return false;

    // Two children: move the in-order successor's payload into z and delete
    // the successor instead, which has no left child.
    if (z->left != nullptr && z->right != nullptr) {
        Node* s = z->right;
        while (s->left != nullptr) s = s->left;
        z->key = s->key;
        z->value = s->value;
        z = s;
    }

    // z has at most one child. Splice it out.
    Node* child = (z->left != nullptr) ? z->left : z->right;
    Node* p = z->parent;
    bool from_left = (p != nullptr && p->left == z);
    replace_child(p, z, child);
    delete z;
    ++m.deallocations;
    --size_;

    // Retrace: the subtree on side `from_left` of p just shrank by one level.
    while (p != nullptr) {
        p->balance = static_cast<std::int8_t>(p->balance + (from_left ? +1 : -1));
        ++m.balance_updates;
        if (p->balance == 1 || p->balance == -1) break;  // height unchanged

        Node* top = p;
        if (p->balance == 2 || p->balance == -2) {
            Node* sibling = (p->balance > 0) ? p->right : p->left;
            const bool height_kept = (sibling->balance == 0);
            top = rebalance(p, m);
            if (height_kept) break;  // rotation kept the subtree height
        }
        // Subtree rooted at `top` is one level shorter: continue upwards.
        p = top->parent;
        if (p != nullptr) from_left = (p->left == top);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Naive (unbalanced) BST baseline
// ---------------------------------------------------------------------------
#include "naive_bst.hpp"

void NaiveBST::insert(int key, int val, TreeMetrics& m) {
    if (root_ == nullptr) {
        root_ = new Node(key, val, nullptr);
        ++m.allocations;
        return;
    }
    Node* p = root_;
    for (;;) {
        ++m.comparisons;
        if (key < p->key) {
            if (p->left == nullptr) { p->left = new Node(key, val, p); break; }
            p = p->left;
        } else if (p->key < key) {
            if (p->right == nullptr) { p->right = new Node(key, val, p); break; }
            p = p->right;
        } else {
            p->value = val;
            return;
        }
    }
    ++m.allocations;
}

bool NaiveBST::find(int key, int& out, TreeMetrics& m) const {
    for (const Node* n = root_; n != nullptr;) {
        ++m.comparisons;
        if (key < n->key) n = n->left;
        else if (n->key < key) n = n->right;
        else { out = n->value; return true; }
    }
    return false;
}

void NaiveBST::clear() noexcept {
    Node* n = root_;
    while (n != nullptr) {
        if (n->left != nullptr) n = n->left;
        else if (n->right != nullptr) n = n->right;
        else {
            Node* p = n->parent;
            if (p != nullptr) { if (p->left == n) p->left = nullptr; else p->right = nullptr; }
            delete n;
            n = p;
        }
    }
    root_ = nullptr;
}

// Iterative height via level-order walk with parent pointers is awkward, so
// use a depth-first walk that tracks depth while moving along parent links.
int NaiveBST::height() const {
    int best = 0, depth = 0;
    const Node* n = root_;
    const Node* prev = nullptr;
    while (n != nullptr) {
        if (prev == n->parent) {          // arrived from above
            ++depth;
            best = std::max(best, depth);
            prev = n;
            if (n->left) n = n->left;
            else if (n->right) n = n->right;
            else { n = n->parent; --depth; }
        } else if (prev == n->left) {     // returned from left subtree
            prev = n;
            if (n->right) n = n->right;
            else { n = n->parent; --depth; }
        } else {                          // returned from right subtree
            prev = n;
            n = n->parent;
            --depth;
        }
    }
    return best;
}
