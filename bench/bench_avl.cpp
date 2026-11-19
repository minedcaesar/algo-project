// bench_avl.cpp -- reproducible benchmark for AVL vs. unbalanced BST vs. std::map.
//
// Output: CSV on stdout, one row per (structure, pattern, n), with the median
// over REPS repetitions of wall-clock time and the (deterministic) operation
// counters from heritage::TreeMetrics (timing via heritage::ScopedTimer).
//
// Usage: bench_avl [max_log2_n=20] [reps=5] [seed=12345]
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>

#include "avl_tree.hpp"
#include "naive_bst.hpp"

namespace {

using heritage::ScopedTimer;
using heritage::TreeMetrics;

enum class Pattern { Random, Ascending, Descending };
const char* name(Pattern p) {
    switch (p) {
        case Pattern::Random: return "random";
        case Pattern::Ascending: return "ascending";
        default: return "descending";
    }
}

// Keys to insert and, separately, a shuffled permutation to look up so that
// lookup cost does not depend on insertion order.
void make_keys(Pattern p, int n, std::mt19937& rng, int* ins, int* look) {
    for (int i = 0; i < n; ++i) ins[i] = 2 * i;  // even keys; odd keys are misses
    if (p == Pattern::Random) std::shuffle(ins, ins + n, rng);
    if (p == Pattern::Descending) std::reverse(ins, ins + n);
    for (int i = 0; i < n; ++i) look[i] = 2 * i;
    std::shuffle(look, look + n, rng);
}

struct Sample {
    TreeMetrics ins, hit, miss;  // elapsed_us filled by ScopedTimer
    AvlStats avl;                // only meaningful for SearchTree
    int height = 0;
};

volatile long long g_sink = 0;  // defeats dead-code elimination

template <class Tree>
Sample run_tree(const int* ins, const int* look, int n) {
    Sample s;
    Tree t;
    long long acc = 0;
    int out = 0;
    {
        ScopedTimer timer(s.ins.elapsed_us);
        for (int i = 0; i < n; ++i) t.insert(ins[i], i, s.ins);
    }
    {
        ScopedTimer timer(s.hit.elapsed_us);
        for (int i = 0; i < n; ++i) acc += t.find(look[i], out, s.hit) ? out : 0;
    }
    {
        ScopedTimer timer(s.miss.elapsed_us);
        for (int i = 0; i < n; ++i) acc += t.find(look[i] + 1, out, s.miss) ? 1 : 0;
    }
    g_sink = g_sink + acc;
    s.height = t.height();
    if constexpr (requires { t.stats(); }) s.avl = t.stats();
    return s;  // t destroyed here, outside the timed regions
}

Sample run_map(const int* ins, const int* look, int n) {
    Sample s;
    std::map<int, int> t;
    long long acc = 0;
    {
        ScopedTimer timer(s.ins.elapsed_us);
        for (int i = 0; i < n; ++i) t[ins[i]] = i;
    }
    {
        ScopedTimer timer(s.hit.elapsed_us);
        for (int i = 0; i < n; ++i) { auto it = t.find(look[i]); acc += it != t.end() ? it->second : 0; }
    }
    {
        ScopedTimer timer(s.miss.elapsed_us);
        for (int i = 0; i < n; ++i) acc += static_cast<long long>(t.count(look[i] + 1));
    }
    g_sink = g_sink + acc;
    s.height = -1;  // not observable through the std::map interface
    return s;
}

double median(double* v, int k) {
    std::sort(v, v + k);
    return (k % 2) ? v[k / 2] : 0.5 * (v[k / 2 - 1] + v[k / 2]);
}

}  // namespace

int main(int argc, char** argv) {
    const int max_log = argc > 1 ? std::atoi(argv[1]) : 20;
    const int reps = argc > 2 ? std::atoi(argv[2]) : 5;
    const unsigned seed = argc > 3 ? static_cast<unsigned>(std::strtoul(argv[3], nullptr, 10)) : 12345u;
    // The naive BST is quadratic on sorted input; cap it to keep runtime sane.
    const int naive_sorted_max_log = 15;

    std::printf("structure,pattern,n,insert_ns,find_hit_ns,find_miss_ns,height,"
                "cmp_per_insert,cmp_per_hit,cmp_per_miss,rot_per_insert,"
                "single_per_insert,double_per_insert,balupd_per_insert\n");

    const int max_n = 1 << max_log;
    int* ins = new int[max_n];
    int* look = new int[max_n];
    double* ti = new double[reps];
    double* th = new double[reps];
    double* tm = new double[reps];

    for (int lg = 10; lg <= max_log; ++lg) {
        const int n = 1 << lg;
        for (Pattern p : {Pattern::Random, Pattern::Ascending, Pattern::Descending}) {
            for (int which = 0; which < 3; ++which) {
                if (which == 1 && p != Pattern::Random && lg > naive_sorted_max_log) continue;
                const char* sname = which == 0 ? "avl" : which == 1 ? "naive_bst" : "std_map";
                Sample last;
                for (int r = 0; r < reps; ++r) {
                    std::mt19937 rng(seed + 1000u * static_cast<unsigned>(lg) + static_cast<unsigned>(r));
                    make_keys(p, n, rng, ins, look);
                    last = which == 0 ? run_tree<SearchTree>(ins, look, n)
                         : which == 1 ? run_tree<NaiveBST>(ins, look, n)
                                      : run_map(ins, look, n);
                    ti[r] = last.ins.elapsed_us * 1000.0 / n;  // ns per operation
                    th[r] = last.hit.elapsed_us * 1000.0 / n;
                    tm[r] = last.miss.elapsed_us * 1000.0 / n;
                }
                // Counters vary across reps only for random input; report the last rep's.
                const double dn = n;
                std::printf("%s,%s,%d,%.2f,%.2f,%.2f,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", sname, name(p), n,
                            median(ti, reps), median(th, reps), median(tm, reps), last.height,
                            last.ins.key_compares / dn, last.hit.key_compares / dn, last.miss.key_compares / dn,
                            last.ins.rotations / dn, last.avl.single_rotations / dn,
                            last.avl.double_rotations / dn, last.avl.balance_updates / dn);
                std::fflush(stdout);
            }
        }
    }
    delete[] ins;
    delete[] look;
    delete[] ti;
    delete[] th;
    delete[] tm;
    return 0;
}
