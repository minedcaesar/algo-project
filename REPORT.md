# AVL Trees (1962) — from a four-page Soviet note to modern kernels

> **Sources.** Statements about the 1962 paper were checked against the
> English translation (scan supplied by the British Library, 5 pp.); quotes
> below are from it. Biographical and "legacy" facts were checked against the
> secondary sources listed in the References. Everything about the code and
> measurements is reproducible from this repository.

---

## 1. History (20%)

### 1.1 Primary source

- **Original (Russian):** Г. М. Адельсон-Вельский, Е. М. Ландис,
  «Один алгоритм организации информации»,
  *Доклады Академии наук СССР*, т. 146, № 2, 1962, с. 263–266.
- **English translation:** G. M. Adel'son-Vel'skii and E. M. Landis,
  "An algorithm for the organization of information",
  *Soviet Mathematics — Doklady*, vol. 3, 1962, pp. 1259–1263,
  translated by Myron J. Ricci. The translation carries the date
  "Received 13/APR/62".

*Doklady* published very short communications; the paper is four pages and
contains: the problem statement, the data layout, a height bound (Lemma 1),
the search procedure, the insertion procedure with a correctness lemma
(Lemma 2) and a three-case restructuring, and a one-paragraph cost argument.
Its bibliography has a **single entry**: P. F. Windley, *Comput. J.* 3
(1960/61), 84 — Windley's "Trees, forests and rearranging", from which the
paper borrows the term "reference board" for the tree index.

### 1.2 Authors and setting

- **Georgy M. Adelson-Velsky** (8 Jan 1922 – 26 Apr 2014). From 1961 he worked
  in Alexander Kronrod's laboratory at the Institute of Theoretical and
  Experimental Physics (ITEP), Moscow, where he co-developed the ITEP chess
  program; it beat the Kotok–McCarthy (Stanford/MIT) program 3–1 in a
  four-game correspondence match in 1966–67, running on an M-20-family
  machine. Its authors were later involved in *Kaissa*, winner of the 1st World
  Computer Chess Championship (Stockholm, 1974).
- **Evgenii M. Landis** (6 Oct 1921 – 12 Dec 1997), mathematician at Moscow
  State University, also a student of Kronrod; known mainly for uniqueness
  theorems and Harnack inequalities for elliptic and parabolic PDEs.

Kronrod is thus the common link between the two authors. Note that **the
paper itself says nothing about chess**; the connection is biographical.

**Problem as stated in the paper.** "The information enters a machine in
sequence from a certain reserve." Each element has an "estimate" (key) and
must be stored "in such a way that at any moment a very large number of
operations is not required to scan the information with the given evaluation
and to record the new information element." The goal is both search and
recording "in C lg N operations". The model is explicitly "a three-address
machine"; no specific computer is named.

The design follows from this model:

- Records are stored **in order of arrival** in one memory area; a separate
  area holds the **"reference board"**, a binary tree acting as an index.
- Each tree cell is one three-address word: address 1 points to the record,
  addresses 2 and 3 to the left and right subordinate cells, and **0 means
  "no child"**. The root ("head") address is kept in a fixed cell *l*.
- The balance information is two extra binary digits per cell (see 1.3).

The alternatives of the period each fail one requirement:

| Representation | Search | Insert |
|---|---|---|
| Sorted array (binary search) | O(log N) | O(N) (shift elements) |
| Unsorted list / array | O(N) | O(1) |
| Plain binary search tree (Windley 1960) | O(log N) average, O(N) worst | same |

### 1.3 Terminology and notation in the paper

| Paper (translation) | Modern term |
|---|---|
| reference board, dyadic tree | binary search tree (as an index) |
| cell; head | node; root |
| "directly subordinate on the left (right)" | left (right) child |
| chain | downward path |
| length of the left (right) branch | height of the left (right) subtree |
| branch pivot | a longest path in a subtree |
| **admissible tree** | **AVL tree**: branch lengths differ by ≤ 1 at every cell |
| estimate | key |
| recorded chain | search path, remembered during insertion |
| separate orders "1 0" / "0 1" / "0 0" | balance −1 (left higher) / +1 (right higher) / 0 |
| "the tree is reorganized" | single or double rotation |

- The balance indicator is literally **two bits**: "If the left branch is
  longer than the right, then 1 0 stands in these orders. If the right branch
  is longer than the left, then 0 1 stands; and if they are equal, then 0 0."
  There is no signed number, so no sign convention; our `balance =
  h(right) − h(left)` maps 1 0 → −1, 0 1 → +1, 0 0 → 0.
- The word **"rotation" does not appear**, nor does the name "AVL". The
  restructuring is given only as pictures (Figures 3–4) of three arrangements.
  The name was in use by 1965 (Foster's paper title, see 1.4).

### 1.4 Curiosities

- **The height bound is stated loosely.** Lemma 1 proves, via
  N_n = N_{n−1} + N_{n−2} + 1 and its closed-form (Fibonacci) solution, that
  n < log_φ(N+1), and then rounds this up to **(3/2) log2(N+1)**. The
  constant 1/log2 φ ≈ 1.4404 is today's textbook bound; the paper's 3/2 is
  just a convenient upper estimate.
- **Comparisons are three-way.** The paper notes that each comparison "gives
  three answers" (<, =, >), which is how our `comparisons` counter is defined.
- **Duplicates are ignored.** "If it turns out that the new element is
  already contained in the former information, the tree does not change."
- **Only insertion and search.** Deletion is not mentioned. The cost argument
  is: search C log2(N+1), walking back up the recorded chain C₁ log2(N+1),
  restructuring a constant — in all, C log2(N+1).
- **No back pointers.** The upward walk uses the *recorded chain*: during the
  search, "remembering the cell addresses of the chain along which we
  proceed". Cells have no parent field.
- It is generally regarded as the **first self-balancing binary search tree**,
  a decade before B-trees (Bayer & McCreight, 1972) and symmetric binary
  B-trees / red–black trees (Bayer 1972; Guibas & Sedgewick 1978).
- **Early Western uptake:** C. C. Foster, "Information storage and retrieval
  using AVL trees", *Proc. ACM 20th National Conference* (1965), 192–205 —
  already using the name "AVL". A 1972 *CACM* note by K. C. Tan ("On Foster's
  information storage and retrieval using AVL trees") showed that a
  construction proposed by Foster does not work in general.
- The worst-case trees (Figure 2 of the paper) are the **Fibonacci trees**:
  N(h) = F(h+2) − 1.

---

## 2. Reconstruction (15%)

### 2.1 Invariant

For every node *x*: `bal(x) = h(x.right) − h(x.left) ∈ {−1, 0, +1}`, where the
empty tree has height 0.

**Height bound.** Let N(h) be the minimum size of an AVL tree of height h.
Then N(0)=0, N(1)=1, N(h)=1+N(h−1)+N(h−2), so N(h)=F(h+2)−1 ≥ φ^h/√5 − 2,
which gives

  h < log_φ(√5 (n+2)) − 2 ≈ **1.4405 · log2(n+2) − 0.3277**.

Hence search and insertion take Θ(log n) in the worst case.

### 2.2 Pseudocode (insertion — the paper's algorithm)

```
INSERT(T, k, v):
    if T.root = NIL: T.root ← new node(k, v); return
    p ← T.root
    loop                                  ▷ 1. ordinary BST descent
        if k < p.key:  if p.left  = NIL: c ← p.left  ← new node(k,v,p); break
                       else p ← p.left
        elif k > p.key: if p.right = NIL: c ← p.right ← new node(k,v,p); break
                       else p ← p.right
        else: p.value ← v; return        ▷ key present (our choice, see 2.4)

    while p ≠ NIL                         ▷ 2. retrace: subtree of c grew by 1
        p.bal ← p.bal + (−1 if c = p.left else +1)
        if p.bal = 0: return              ▷ height of p unchanged
        if |p.bal| = 1: c ← p; p ← p.parent; continue
        REBALANCE(p); return              ▷ |p.bal| = 2: fix and stop

REBALANCE(x):                             ▷ x.bal = ±2
    if x.bal = −2:
        if x.left.bal = +1: ROTATE-LEFT(x.left)     ▷ double (LR)
        ROTATE-RIGHT(x)                              ▷ single (LL) or 2nd half
    else:
        if x.right.bal = −1: ROTATE-RIGHT(x.right)  ▷ double (RL)
        ROTATE-LEFT(x)

ROTATE-LEFT(x):  y ← x.right; x.right ← y.left; y.left ← x; (fix parents)
    x.bal ← x.bal − 1 − max(y.bal, 0)
    y.bal ← y.bal − 1 + min(x.bal, 0)        ▷ uses the *new* x.bal
ROTATE-RIGHT(x): mirror image
    x.bal ← x.bal + 1 − min(y.bal, 0)
    y.bal ← y.bal + 1 + max(x.bal, 0)
```

Key property (in the paper): after an insertion **at most one** single or
double rotation is needed, because the rotated subtree regains exactly its
pre-insertion height, so no ancestor's balance changes.

**Correspondence with the paper.** The paper phrases step 2 as: correct cell
*u* (the new leaf's parent); if *u* was unbalanced it becomes 0 0 and
recording is complete. Otherwise take the maximal run 𝔈 of the recorded chain,
going up from *u*, whose cells had 0 0; set each to 1 0 or 0 1 (Lemma 2
proves the new leaf lies on the longest path of each). Let *s₀* be the cell
just above 𝔈. Three cases: (1) 𝔈 is the whole chain — done; (2) 𝔈 is on
*s₀*'s short branch — set *s₀* to 0 0; (3) 𝔈 is on *s₀*'s long branch —
reorganize per Figure 4. Figure 3's arrangement **a** is our single rotation,
**b** our double rotation (with the middle cell's two possible balances, "1 0
or 0 0" / "0 0 or 0 1" in Figure 4), and **c** the three-cell special case of
the double rotation where all subtrees are empty. Our loop performs the same
updates bottom-up, one cell per iteration; the run 𝔈 is exactly the cells
the loop passes with |bal| becoming 1.

### 2.3 Deletion (extension, not in the paper)

Standard approach: delete as in a BST (replacing a two-child node by its
in-order successor), then retrace upward. Unlike insertion, a rotation can
*shorten* the subtree, so retracing may continue to the root: up to
O(log n) rotations per deletion. A rotation whose sibling has balance 0
preserves height and stops the retrace.

### 2.4 Ambiguities and our resolutions

| Issue | Paper | Our decision |
|---|---|---|
| Duplicate keys | "the tree does not change" (record not replaced) | Overwrite the stored value; shape unchanged (the course interface pairs a key with a value, so updating is the useful behaviour) — **deviation** |
| Retrace mechanism | Remembered list of addresses of the search path ("recorded chain"); cells have no parent field | Parent pointers (required by the course `Node`) — **deviation** |
| Balance updates | Given only as figures (new values in brackets in Figure 4) | Generic formulas valid for any balances, so the same rotations serve deletion |
| Balance encoding | Two bits: 1 0 / 0 1 / 0 0 | `int8_t`, `h(right) − h(left)` ∈ {−1, 0, +1} |
| Height bound | (3/2) log2(N+1), rounded from log_φ(N+1) | We check the tight form 1.4405 log2(n+2) − 0.3277 in tests |
| Head (root) change | "necessary to change again the fixed cell in which the head … is indicated" | `replace_child` updates `root_` when the rotated node had no parent |
| Deletion | Absent | Implemented as an extension and labelled as such |

### 2.5 Implementation decisions introduced by C++

- `Node` is the course struct plus `std::int8_t balance` (the paper's
  two-bit indicator; 1 byte, padding makes the node 32 bytes on x86-64
  either way).
- Raw `new`/`delete` only; no `std::vector` / smart pointers in the structure.
  `clear()` and the destructor delete nodes **iteratively** in post-order via
  parent pointers (no recursion, no auxiliary stack).
- `inorder()` walks the tree using parent pointers: O(n) time, O(1) extra space;
  it writes `min(size, cap)` keys.
- Rule of five: deep copy (copy-and-swap, strong exception guarantee), and
  pointer-stealing move.
- Everything except the deep copy is iterative; copy recursion depth is
  bounded by the AVL height.
- `metrics.hpp` is a **stand-in** for the Moodle header (comparisons,
  rotations, single/double rebalancings, balance updates, allocations).

---

## 3. Evaluation (10%)

### 3.1 Theoretical costs

| Operation | Worst case | Notes |
|---|---|---|
| `find` | Θ(log n), ≤ 1.4405 log2(n+2) comparisons | height bound §2.1 |
| `insert` | Θ(log n) | ≤ 1 rebalancing (single or double rotation) |
| `erase` (ext.) | Θ(log n) | up to O(log n) rotations |
| `inorder` | Θ(n) time, O(1) extra space | parent-pointer walk |
| `clear` / destructor | Θ(n) time, O(1) extra space | |
| Space | n nodes × 32 bytes | 4 pointers/ints + 1-byte balance + padding |

Baselines: an unbalanced BST costs Θ(n) per operation on sorted input
(Θ(n²) to build), and ≈ 2 ln n ≈ 1.386 log2 n comparisons per successful search
on random input. `std::map` (libstdc++) is a red–black tree: height
≤ 2 log2(n+1).

### 3.2 Methodology

- `bench/bench_avl.cpp`, run by `scripts/run_bench.sh 20 5 12345`.
- n = 2^10 … 2^20; distinct even keys inserted in **random**, **ascending** or
  **descending** order; then n successful lookups (all keys, shuffled) and n
  unsuccessful lookups (odd keys). Fixed seeds: identical inputs per run.
- Times are the **median of 5 repetitions**, in ns per operation; the counters
  (comparisons, rotations, …) are deterministic per input.
- The unbalanced BST on sorted input is capped at n = 2^15 (quadratic).
- Environment (`results/env.txt`): Intel Xeon @ 2.10 GHz, 4 vCPUs, Linux 6.18,
  g++ 13.3, `-O2`. Cloud VM: absolute times are noisy (±10–20%); trends and
  counters are what matter.

### 3.3 Results

**Height** (`results/height.png`)

| n | AVL random | AVL sorted | bound 1.4405 log2(n+2)−0.33 | ⌈log2(n+1)⌉ | naive BST random |
|---|---|---|---|---|---|
| 2^10 | 12 | 11 | 14.1 | 11 | 22 |
| 2^15 | 18 | 16 | 21.3 | 16 | 37 |
| 2^20 | 24 | 21 | 28.5 | 21 | 50 |

Random AVL trees sit about 1.2·log2 n — well below the worst-case Fibonacci
bound, which is only reached by adversarially constructed trees. Sorted input,
the plain BST's worst case, yields *nearly perfect* AVL trees: every insert
into the rightmost path triggers exactly one single rotation that keeps the
tree complete.

**Comparisons per operation** (`results/comparisons.png`), n = 2^20:

| | successful find | unsuccessful find | theory |
|---|---|---|---|
| AVL, random | 19.42 | 20.42 | ≈ log2 n = 20 |
| AVL, sorted | 19.00 | 20.00 | complete tree: log2 n − 1 + ε |
| Naive BST, random | 25.89 | 26.89 | 2(1+1/n)H_n − 3 = **25.88** |
| Naive BST, sorted (n=2^15) | 16384.5 | 16385.5 | (n+1)/2 |

The naive-BST random case matches the classical average-case formula to two
decimals, which is a good check that the counter is correct. AVL search is
~25% cheaper in comparisons than a random BST, and exponentially cheaper than
a degenerate one.

**Rebalancing work per insertion** (constant in n, measured n = 2^10 … 2^20):

| Input | rebalancings / insert | single | double | elementary rotations | balance updates |
|---|---|---|---|---|---|
| random | 0.466 | 0.233 | 0.233 | 0.698 | 2.79 |
| sorted | 1.000 | 1.000 | 0 | 1.000 | 3.00 |

About **47% of random insertions need a rebalancing**, split evenly between
single and double rotations; retracing touches < 3 ancestors on average. So
the extra work over a plain BST is O(1) per insertion — the paper's "at most
one restructuring" is in practice "one restructuring about half the time".
(Knuth, TAOCP Vol. 3 §6.2.3, reports empirical statistics for random AVL
insertions; we have not compared against his figures numerically.)

**Time** (ns/op, median; `results/insert_time.png`, `results/find_time.png`)

| n | AVL insert rnd | std::map insert rnd | naive insert rnd | AVL insert sorted | naive insert sorted |
|---|---|---|---|---|---|
| 2^10 | 86 | 88 | 70 | 22 | 881 |
| 2^15 | 175 | 177 | 172 | 36 | 200 803 |
| 2^20 | 1174 | 1114 | 1234 | 191 | — (quadratic) |

| n | AVL find-hit rnd | std::map find-hit rnd | naive find-hit rnd |
|---|---|---|---|
| 2^10 | 52 | 55 | 56 |
| 2^15 | 143 | 151 | 166 |
| 2^20 | 1097 | 1236 | 1425 |

### 3.4 Do measurements agree with theory?

- **Counters: yes, closely.** Comparisons grow as log2 n with slope 1 for AVL
  and ≈1.39 for the random BST; height stays inside the proven bound;
  rotations per insert are constant.
- **Wall-clock: only in shape for small n.** From 2^15 to 2^20, log2 n grows
  by 33% but time per random insert grows ~6.7×. The cause is the **memory
  hierarchy**, not the algorithm: at 2^20 nodes the tree occupies ~32 MB and
  each of the ~20 levels of a search is a likely cache miss (pointer chasing
  through nodes allocated in random order). The comparison count is the same
  log n, but each step costs a DRAM access instead of an L1 hit.
- **Sorted input is 5–6× faster than random** for the same number of
  comparisons because every insertion walks the same, cache-hot right spine.
  Lookups were shuffled, so find times do not show this effect.
- **AVL vs. red–black (`std::map`)**: within noise for insertion; AVL lookups
  are ~10% faster at 2^20, consistent with AVL's tighter height bound
  (1.44 log2 n vs. 2 log2 n; `std::map`'s actual height is not observable, so
  this explanation is plausible but not proven here) — the classic "AVL for
  read-heavy workloads" trade-off. `std::map` also pays for its generic
  allocator and node layout, so this is not a pure algorithm comparison.
- **Unbalanced BST on random input** is competitive in time at small n (no
  balancing overhead) but loses ~30% on lookups at 2^20 — and on sorted input
  it is ~5 500× slower at only 2^15 keys, the exact failure the 1962 paper set
  out to fix.

### 3.5 Constant factors that matter

1. Cache misses per level dominate everything above ~10^5 nodes.
2. Node size (32 bytes): the 1-byte balance indicator costs nothing extra due
   to alignment; storing heights instead would also fit.
3. Parent pointers add 8 bytes/node and a few writes per rotation, in exchange
   for a stack-free retrace, O(1)-space traversal and iterative `clear()`.
4. Rebalancing (≈0.7 rotations, ≈2.8 indicator updates per insert) is cheap
   relative to the descent.

### 3.6 Limitations

- Single machine, cloud VM, no CPU pinning; no hardware counters (perf) for
  cache misses — the cache explanation is inferred from the scaling, not
  measured directly.
- `erase` is tested but not benchmarked.
- Height of `std::map` is not observable through its interface.

---

## 4. Legacy (5%)

- **Direct descendants in production code** (checked against current
  source/documentation):
  - **OpenZFS** (from Solaris/illumos, also in FreeBSD): generic intrusive AVL
    library `include/sys/avl.h`, `module/avl/avl.c`, used throughout ZFS.
  - **Windows kernel** (`ntddk.h`): `RTL_AVL_TABLE` and
    `RtlInsertElementGenericTableAvl` & co. Microsoft's documentation notes
    generic tables default to splay trees and describes the AVL variant as
    more balanced and shallower; duplicates are rejected — as in the 1962
    paper.
  - **LLVM** `llvm/ADT/ImmutableSet.h`: `ImutAVLTree`, a persistent
    (functional) AVL tree behind `ImmutableSet`/`ImmutableMap`, used e.g. by
    the Clang static analyzer.
  - **OCaml** standard library `Set`/`Map`: "balanced binary trees (the
    heights of the children differ by at most 2)" — a relaxed AVL invariant
    trading a little height for fewer rotations.
  - GNU libavl (B. Pfaff): reference C implementations of AVL and related trees.
- **Variants that changed the trade-off.**
  - Red–black trees (1972/1978): weaker balance (height ≤ 2 log2(n+1)) but
    O(1) amortized rotations on deletion; became the default in `std::map`
    (libstdc++), Java `TreeMap`, and the Linux kernel `rbtree`.
  - B-trees (1972): the same "keep it balanced by local repair" idea for disks;
    the backbone of databases and file systems.
  - Weak AVL / rank-balanced trees (Haeupler, Sen, Tarjan, 2015): AVL height
    on insertion-only workloads with red–black-like O(1) amortized rebalancing.
- **What remains relevant.** The balance-indicator idea (a few bits per node,
  local repair, Fibonacci worst case) and the rotation primitive are now
  universal; AVL is still preferred when lookups dominate updates because its
  height bound (≈1.44 log2 n) is tighter than red–black's (≈2 log2 n).

---

## References

1. Adel'son-Vel'skii, G. M., Landis, E. M. (1962). An algorithm for the
   organization of information. *Soviet Math. Doklady* 3, 1259–1263,
   transl. M. J. Ricci.
   (Original: *Doklady AN SSSR* 146(2), 263–266.)
2. Knuth, D. E. (1998). *The Art of Computer Programming, Vol. 3: Sorting and
   Searching*, 2nd ed., §6.2.3 "Balanced Trees". Addison-Wesley.
3. Cormen, T. H. et al. (2022). *Introduction to Algorithms*, 4th ed. MIT Press
   (Problem 13-3, AVL trees).
4. Foster, C. C. (1965). Information storage and retrieval using AVL trees.
   *Proc. ACM 20th National Conference*, 192–205.
4b. Tan, K. C. (1972). On Foster's information storage and retrieval using
   AVL trees. *Communications of the ACM* 15(9), 843.
4c. Windley, P. F. (1960). Trees, forests and rearranging. *The Computer
   Journal* 3(2), 84–88. (The paper's only cited reference.)
5. Guibas, L. J., Sedgewick, R. (1978). A dichromatic framework for balanced
   trees. *FOCS 1978*, 8–21.
6. Bayer, R. (1972). Symmetric binary B-trees: data structure and maintenance
   algorithms. *Acta Informatica* 1, 290–306.
7. Haeupler, B., Sen, S., Tarjan, R. E. (2015). Rank-balanced trees.
   *ACM Transactions on Algorithms* 11(4), 30.
8. Biographical sources: Wikipedia articles "Georgy Adelson-Velsky" and
   "Evgenii Landis"; Chess Programming Wiki, "Georgy Adelson-Velsky", "ITEP
   Chess Program", "Kaissa".
9. Implementation sources: OpenZFS `include/sys/avl.h`; Microsoft Learn,
   `RtlInsertElementGenericTableAvl` / `RTL_AVL_TABLE`; LLVM
   `llvm/ADT/ImmutableSet.h`; OCaml `stdlib/set.ml`.
