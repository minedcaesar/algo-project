# AVL tree — Algorithmic Heritage Initiative (ADS 2026–27, Trees line)

AVL tree (Adelson-Velsky & Landis, 1962) in C++20, behind the course's
`SearchTree` / `Node` interface, with raw pointers only.

## Layout
| Path | What |
|---|---|
| `include/avl_tree.hpp`, `src/avl_tree.cpp` | `Node`, `SearchTree` (insert / find / inorder / clear, plus erase, size, height) |
| `include/metrics.hpp` | Official course header (`heritage::TreeMetrics`, `heritage::ScopedTimer`), unmodified |
| `include/naive_bst.hpp` | Unbalanced BST baseline (benchmark only) |
| `tests/test_avl.cpp` | Invariant, oracle (`std::map`), edge-case and rotation tests |
| `bench/bench_avl.cpp`, `scripts/run_bench.sh`, `scripts/plot.py` | Benchmark + optional plots |

## Build & run
Requires g++ ≥ 12.
```sh
make test     # -std=c++20 -O2 -Wall -Wextra -Werror
make asan     # same tests under AddressSanitizer + UBSan (leak check)
make bench    # writes results/bench.csv, results/env.txt (+ PNGs if matplotlib is installed)
scripts/run_bench.sh 16 3   # smaller/faster run: max n = 2^16, 3 reps
```

## Results
The committed `results/` folder holds a full run (n up to 2^20, 5 reps) with
the machine description in `results/env.txt`. Analysis is in `REPORT.md` §3.
A full run takes ~10 minutes, almost all of it spent on the unbalanced BST
with sorted input (quadratic, capped at n = 2^15).
