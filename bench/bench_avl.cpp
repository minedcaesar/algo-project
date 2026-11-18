// bench_avl.cpp -- reproducible benchmark for AVL vs. unbalanced BST vs. std::map.
//
// Output: CSV on stdout, one row per (structure, pattern, n), with the median
// over REPS repetitions of wall-clock time and the (deterministic) operation
// counters from TreeMetrics.
//
// Usage: bench_avl [max_log2_n=20] [reps=5] [seed=12345]
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>

#include "avl_tree.hpp"
#include "naive_bst.hpp"

namespace {

using Clock = std::chrono::steady_clock;

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
    double insert_ns = 0, find_hit_ns = 0, find_miss_ns = 0;
    TreeMetrics ins, hit, miss;
    int height = 0;
};

volatile long long g_sink = 0;  // defeats dead-code elimination

template <class Tree>
Sample run_tree(const int* ins, const int* look, int n) {
    Sample s;
    Tree t;
    auto t0 = Clock::now();
    for (int i = 0; i < n; ++i) t.insert(ins[i], i, s.ins);
    auto t1 = Clock::now();
    long long acc = 0;
    int out = 0;
    for (int i = 0; i < n; ++i) acc += t.find(look[i], out, s.hit) ? out : 0;
    auto t2 = Clock::now();
    for (int i = 0; i < n; ++i) acc += t.find(look[i] + 1, out, s.miss) ? 1 : 0;
    auto t3 = Clock::now();
    g_sink = g_sink + acc;
    s.height = t.height();
    auto ns = [](auto a, auto b) { return std::chrono::duration<double, std::nano>(b - a).count(); };
    s.insert_ns = ns(t0, t1) / n;
    s.find_hit_ns = ns(t1, t2) / n;
    s.find_miss_ns = ns(t2, t3) / n;
    return s;  // t destroyed here, outside the timed region
}

Sample run_map(const int* ins, const int* look, int n) {
    Sample s;
    std::map<int, int> t;
    auto t0 = Clock::now();
    for (int i = 0; i < n; ++i) t[ins[i]] = i;
    auto t1 = Clock::now();
    long long acc = 0;
    for (int i = 0; i < n; ++i) { auto it = t.find(look[i]); acc += it != t.end() ? it->second : 0; }
    auto t2 = Clock::now();
    for (int i = 0; i < n; ++i) acc += t.count(look[i] + 1);
    auto t3 = Clock::now();
    g_sink = g_sink + acc;
    auto ns = [](auto a, auto b) { return std::chrono::duration<double, std::nano>(b - a).count(); };
    s.insert_ns = ns(t0, t1) / n;
    s.find_hit_ns = ns(t1, t2) / n;
    s.find_miss_ns = ns(t2, t3) / n;
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
                    ti[r] = last.insert_ns;
                    th[r] = last.find_hit_ns;
                    tm[r] = last.find_miss_ns;
                }
                // Counters vary across reps only for random input; report the last rep's.
                const double dn = n;
                std::printf("%s,%s,%d,%.2f,%.2f,%.2f,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", sname, name(p), n,
                            median(ti, reps), median(th, reps), median(tm, reps), last.height,
                            last.ins.comparisons / dn, last.hit.comparisons / dn, last.miss.comparisons / dn,
                            last.ins.rotations / dn, last.ins.single_rotations / dn,
                            last.ins.double_rotations / dn, last.ins.balance_updates / dn);
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
