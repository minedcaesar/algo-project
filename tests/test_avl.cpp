// test_avl.cpp -- correctness tests for the AVL tree.
//
// Oracles used:
//   * structural invariants checked on every node (order, parent links,
//     stored balance == real height difference, |balance| <= 1, size);
//   * the Fibonacci height bound h < 1.4405 log2(n+2) - 0.3277;
//   * std::map as a trusted reference for randomized operation sequences
//     (std::map is used only here as an oracle, never inside the tree);
//   * hand-checked small trees for each of the four rotation cases.
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>
#include <utility>

#include "avl_tree.hpp"

using heritage::TreeMetrics;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failures;                                                        \
            std::fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                        \
    } while (0)

// ---------------------------------------------------------------------------
// Invariant checker
// ---------------------------------------------------------------------------
struct CheckResult {
    bool ok = true;
    std::size_t count = 0;
};

// Returns subtree height; records any violation in r.
static int check_node(const Node* n, const Node* parent, long long lo, long long hi, CheckResult& r) {
    if (n == nullptr) return 0;
    ++r.count;
    if (n->parent != parent) r.ok = false;
    if (!(lo < n->key && n->key < hi)) r.ok = false;  // strict: keys are unique
    const int hl = check_node(n->left, n, lo, n->key, r);
    const int hr = check_node(n->right, n, n->key, hi, r);
    if (n->balance != hr - hl) r.ok = false;
    if (n->balance < -1 || n->balance > 1) r.ok = false;
    return 1 + (hl > hr ? hl : hr);
}

static bool valid_avl(const SearchTree& t) {
    CheckResult r;
    const int h = check_node(t.root(), nullptr, LLONG_MIN, LLONG_MAX, r);
    if (r.count != t.size()) return false;
    if (h != t.height()) return false;
    if (t.size() > 0) {
        const double bound = 1.4405 * std::log2(static_cast<double>(t.size()) + 2.0) - 0.3277;
        if (h >= bound) return false;
    }
    return r.ok;
}

static bool matches_oracle(const SearchTree& t, const std::map<int, int>& ref) {
    if (t.size() != ref.size()) return false;
    int* buf = new int[ref.size() + 1];
    t.inorder(buf, ref.size());
    bool ok = true;
    std::size_t i = 0;
    TreeMetrics m;
    for (const auto& [k, v] : ref) {
        int got = 0;
        if (buf[i++] != k) ok = false;
        if (!t.find(k, got, m) || got != v) ok = false;
    }
    delete[] buf;
    return ok;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------
static void test_empty() {
    SearchTree t;
    TreeMetrics m;
    int out = 12345;
    CHECK(t.empty());
    CHECK(t.height() == 0);
    CHECK(!t.find(0, out, m));
    CHECK(out == 12345);  // out untouched on miss
    CHECK(m.key_compares == 0);
    int buf[1] = {777};
    t.inorder(buf, 1);
    CHECK(buf[0] == 777);
    CHECK(!t.erase(5, m));
    t.clear();
    CHECK(valid_avl(t));
}

static void test_single_and_duplicates() {
    SearchTree t;
    TreeMetrics m;
    t.insert(7, 70, m);
    CHECK(t.size() == 1 && t.height() == 1);
    t.insert(7, 71, m);  // duplicate key: value overwritten, no new node
    CHECK(t.size() == 1);
    CHECK(m.node_allocs == 1);
    int out = 0;
    CHECK(t.find(7, out, m) && out == 71);
    CHECK(valid_avl(t));
}

// Each of the four textbook cases on three keys must end with key 2 at the
// root, keys 1 and 3 as children, all balances 0, after exactly 1 or 2
// rotations.
static void rotation_case(int a, int b, int c, std::uint64_t expect_rot, bool expect_double) {
    SearchTree t;
    TreeMetrics m;
    t.insert(a, 0, m);
    t.insert(b, 0, m);
    t.insert(c, 0, m);
    const Node* r = t.root();
    CHECK(r != nullptr && r->key == 2);
    CHECK(r->left != nullptr && r->left->key == 1);
    CHECK(r->right != nullptr && r->right->key == 3);
    CHECK(r->balance == 0 && r->left->balance == 0 && r->right->balance == 0);
    CHECK(m.rotations == expect_rot);
    CHECK(t.stats().double_rotations == (expect_double ? 1u : 0u));
    CHECK(t.stats().single_rotations == (expect_double ? 0u : 1u));
    CHECK(valid_avl(t));
}

static void test_rotations() {
    rotation_case(3, 2, 1, 1, false);  // left-left   -> single right
    rotation_case(1, 2, 3, 1, false);  // right-right -> single left
    rotation_case(3, 1, 2, 2, true);   // left-right  -> double
    rotation_case(1, 3, 2, 2, true);   // right-left  -> double
}

// Double rotation where the middle node has a non-zero balance, which checks
// the less obvious balance-update branches.
static void test_double_rotation_nonzero_middle() {
    SearchTree t;
    TreeMetrics m;
    for (int k : {50, 20, 70, 10, 30, 25}) t.insert(k, k, m);  // LR with middle left-heavy
    CHECK(valid_avl(t));
    CHECK(t.root()->key == 30);
    SearchTree u;
    for (int k : {50, 20, 70, 10, 30, 35}) u.insert(k, k, m);  // LR with middle right-heavy
    CHECK(valid_avl(u));
    CHECK(u.root()->key == 30);
}

static void test_extreme_keys() {
    SearchTree t;
    TreeMetrics m;
    t.insert(INT_MAX, 1, m);
    t.insert(INT_MIN, 2, m);
    t.insert(0, 3, m);
    t.insert(-1, 4, m);
    t.insert(INT_MAX - 1, 5, m);
    CHECK(valid_avl(t));
    int out = 0;
    CHECK(t.find(INT_MIN, out, m) && out == 2);
    CHECK(t.find(INT_MAX, out, m) && out == 1);
    CHECK(!t.find(1, out, m));
    int buf[5];
    t.inorder(buf, 5);
    CHECK(buf[0] == INT_MIN && buf[1] == -1 && buf[2] == 0 && buf[3] == INT_MAX - 1 && buf[4] == INT_MAX);
}

static void test_inorder_cap() {
    SearchTree t;
    TreeMetrics m;
    for (int i = 10; i >= 1; --i) t.insert(i, i, m);
    int buf[12];
    for (int& x : buf) x = -1;
    t.inorder(buf, 4);  // cap smaller than size: only the 4 smallest keys
    CHECK(buf[0] == 1 && buf[1] == 2 && buf[2] == 3 && buf[3] == 4);
    CHECK(buf[4] == -1);
    t.inorder(buf, 12);  // cap larger than size: writes exactly size() keys
    for (int i = 0; i < 10; ++i) CHECK(buf[i] == i + 1);
    CHECK(buf[10] == -1);
    t.inorder(nullptr, 5);  // must not crash
    t.inorder(buf, 0);
}

// Sorted input degenerates a plain BST into a list; AVL must stay perfectly
// balanced: inserting 2^k - 1 ascending keys yields a complete tree.
static void test_sorted_inputs() {
    for (int dir = 0; dir < 2; ++dir) {
        SearchTree t;
        TreeMetrics m;
        const int n = (1 << 12) - 1;
        for (int i = 0; i < n; ++i) t.insert(dir == 0 ? i : n - i, i, m);
        CHECK(valid_avl(t));
        CHECK(t.height() == 12);
    }
}

// Minimal (Fibonacci) AVL trees are the worst case for height. Building one
// by insertion is hard, so instead check the bound is tight-ish on random data
// and that the counts N(h) = F(h+2) - 1 are respected on all observed trees.
static void test_height_bound_random() {
    std::mt19937 rng(42);
    SearchTree t;
    TreeMetrics m;
    for (int i = 0; i < 200000; ++i) t.insert(static_cast<int>(rng()), i, m);
    CHECK(valid_avl(t));
    // minimum nodes for height h: F(h+2) - 1
    long long f0 = 0, f1 = 1;  // F(0), F(1)
    for (int i = 0; i < t.height() + 2; ++i) { long long f2 = f0 + f1; f0 = f1; f1 = f2; }
    CHECK(static_cast<long long>(t.size()) >= f0 - 1);
}

static void test_randomized_vs_oracle() {
    std::mt19937 rng(2026);
    for (int round = 0; round < 30; ++round) {
        SearchTree t;
        std::map<int, int> ref;
        TreeMetrics m;
        const int range = (round % 3 == 0) ? 50 : 5000;  // small range => many duplicates/erases
        const int ops = 4000;
        bool ok = true;
        for (int i = 0; i < ops; ++i) {
            const int key = static_cast<int>(rng() % range) - range / 2;
            const unsigned op = rng() % 10;
            if (op < 6) {
                t.insert(key, i, m);
                ref[key] = i;
            } else if (op < 9) {
                const bool erased = t.erase(key, m);
                if (erased != (ref.erase(key) == 1)) ok = false;
            } else {
                int out = 0;
                auto it = ref.find(key);
                const bool found = t.find(key, out, m);
                if (found != (it != ref.end())) ok = false;
                if (found && it != ref.end() && out != it->second) ok = false;
            }
            if (i % 97 == 0 && !valid_avl(t)) ok = false;
        }
        CHECK(ok);
        CHECK(valid_avl(t));
        CHECK(matches_oracle(t, ref));
    }
}

static void test_erase_all_orders() {
    // Every deletion position for n <= 9 keys, inserted ascending, descending
    // and in a shuffled order.
    for (int n = 1; n <= 9; ++n) {
        for (int victim = 0; victim < n; ++victim) {
            for (int order = 0; order < 3; ++order) {
                SearchTree t;
                TreeMetrics m;
                int keys[9];
                for (int i = 0; i < n; ++i) keys[i] = order == 1 ? n - 1 - i : i;
                if (order == 2) {
                    std::mt19937 rng(static_cast<unsigned>(n * 31 + victim));
                    std::shuffle(keys, keys + n, rng);
                }
                for (int i = 0; i < n; ++i) t.insert(keys[i], keys[i], m);
                CHECK(t.erase(victim, m));
                CHECK(!t.erase(victim, m));
                CHECK(t.size() == static_cast<std::size_t>(n - 1));
                CHECK(valid_avl(t));
                int out = 0;
                CHECK(!t.find(victim, out, m));
            }
        }
    }
    // Drain a large tree completely.
    SearchTree t;
    TreeMetrics m;
    std::mt19937 rng(7);
    for (int i = 0; i < 5000; ++i) t.insert(i, i, m);
    int* keys = new int[5000];
    for (int i = 0; i < 5000; ++i) keys[i] = i;
    std::shuffle(keys, keys + 5000, rng);
    bool ok = true;
    for (int i = 0; i < 5000; ++i) {
        if (!t.erase(keys[i], m)) ok = false;
        if (i % 251 == 0 && !valid_avl(t)) ok = false;
    }
    delete[] keys;
    CHECK(ok);
    CHECK(t.empty() && t.root() == nullptr);
    CHECK(t.stats().node_frees == 5000);
}

static void test_copy_move_clear() {
    SearchTree a;
    TreeMetrics m;
    for (int i = 0; i < 1000; ++i) a.insert(i * 3 % 1000, i, m);
    SearchTree b(a);  // deep copy
    CHECK(valid_avl(b));
    CHECK(b.size() == a.size());
    CHECK(b.root() != a.root());
    a.insert(5000, 1, m);
    int out = 0;
    CHECK(!b.find(5000, out, m));  // independent

    SearchTree c;
    c.insert(-1, -1, m);
    c = b;  // copy assignment replaces old content
    CHECK(!c.find(-1, out, m));
    CHECK(valid_avl(c) && c.size() == 1000);
    const SearchTree& alias = c;
    c = alias;  // self-assignment
    CHECK(valid_avl(c) && c.size() == 1000);

    SearchTree d(std::move(c));
    CHECK(c.empty() && c.root() == nullptr);
    CHECK(valid_avl(d) && d.size() == 1000);

    SearchTree e;
    e.insert(1, 1, m);
    e = std::move(d);
    CHECK(d.empty());
    CHECK(e.size() == 1000 && valid_avl(e));

    e.clear();
    CHECK(e.empty() && e.height() == 0 && e.root() == nullptr);
    e.insert(42, 1, m);  // reusable after clear
    CHECK(e.size() == 1 && valid_avl(e));
}

static void test_metrics_counting() {
    SearchTree t;
    TreeMetrics m;
    for (int i = 0; i < 7; ++i) t.insert(i, i, m);  // perfect tree of height 3
    CHECK(m.node_allocs == 7);
    // Inserting 4 into 1(0, 2(-, 3)) places it at depth 4 before the rotation.
    CHECK(m.max_depth == 4);
    CHECK(m.peak_aux_bytes == 0);  // no auxiliary memory is ever used
    m.reset();
    int out = 0;
    CHECK(t.find(3, out, m));  // root
    CHECK(m.key_compares == 1);
    CHECK(m.max_depth == 1);
    m.reset();
    CHECK(t.find(0, out, m));  // leaf at depth 3
    CHECK(m.key_compares == 3);
    CHECK(m.max_depth == 3);
    m.reset();
    CHECK(!t.find(100, out, m));  // miss: walks one full path
    CHECK(m.key_compares == 3);
    CHECK(m.max_depth == 3);
    CHECK(m.rotations == 0);  // lookups never restructure

    // Stats: ascending insertion of 7 keys = 4 single rotations, and the
    // copy starts with fresh stats while a move carries them over.
    CHECK(t.stats().single_rotations == 4 && t.stats().double_rotations == 0);
    SearchTree c(t);
    CHECK(c.stats().single_rotations == 0);
    SearchTree mv(std::move(t));
    CHECK(mv.stats().single_rotations == 4 && t.stats().single_rotations == 0);
    mv.reset_stats();
    CHECK(mv.stats().balance_updates == 0);
}

int main() {
    struct { const char* name; void (*fn)(); } tests[] = {
        {"empty tree", test_empty},
        {"single node and duplicate keys", test_single_and_duplicates},
        {"four rotation cases", test_rotations},
        {"double rotation, non-zero middle", test_double_rotation_nonzero_middle},
        {"extreme keys", test_extreme_keys},
        {"inorder capacity handling", test_inorder_cap},
        {"sorted inputs stay balanced", test_sorted_inputs},
        {"height bound on 200k random keys", test_height_bound_random},
        {"randomized ops vs std::map", test_randomized_vs_oracle},
        {"erase: exhaustive small + drain", test_erase_all_orders},
        {"copy / move / clear", test_copy_move_clear},
        {"metrics counting", test_metrics_counting},
    };
    for (const auto& t : tests) {
        const int before = g_failures;
        t.fn();
        std::printf("[%s] %s\n", g_failures == before ? " OK " : "FAIL", t.name);
    }
    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
