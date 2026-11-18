# AVL Trees (1962) — from a four-page Soviet note to modern kernels

> **Status of this draft.** Items marked **⚠ verify** are drawn from secondary
> knowledge and must be checked against the primary source (the Russian
> original or the English translation) before submission. Everything about the
> code and measurements is reproducible from this repository.

---

## 1. History (20%)

### 1.1 Primary source

- **Original (Russian):** Г. М. Адельсон-Вельский, Е. М. Ландис,
  «Один алгоритм организации информации»,
  *Доклады Академии наук СССР*, т. 146, № 2, 1962, с. 263–266.
- **English translation:** G. M. Adel'son-Vel'skii and E. M. Landis,
  "An algorithm for the organization of information",
  *Soviet Mathematics — Doklady*, vol. 3, 1962, pp. 1259–1263
  (translated by Myron J. Ricci). **⚠ verify page range and translator.**

*Doklady* published short communications (a few pages) presented by an
Academician; this explains the paper's density: definitions, the insertion
procedure, and the height bound, with almost no discussion.

### 1.2 Authors and setting

- **Georgy M. Adelson-Velsky** (1922–2014) worked in Moscow on early computing
  and is best known outside this paper for computer chess: he co-authored the
  ITEP program that played the 1966–67 correspondence match against the
  Stanford (Kotok–McCarthy) program, and later the program *Kaissa*, winner of
  the first World Computer Chess Championship (1974). **⚠ verify dates.**
- **Evgenii M. Landis** (1921–1997), a mathematician at Moscow State
  University, known mainly for work in partial differential equations.

The motivating problem, as stated in the title, is the *organization of
information*: maintaining a set of records in the memory of a computer so that
both **lookup** and **insertion of a new record** are cheap. The two known
options of the time each fail one of these:

| Representation | Search | Insert |
|---|---|---|
| Sorted array (binary search) | O(log N) | O(N) (shift elements) |
| Unsorted list / array | O(N) | O(1) |
| Plain binary search tree | O(log N) *on average*, O(N) worst | same |

The paper's contribution is a tree whose *worst case* for both operations is
O(log N), by keeping a small amount of balance information per node and
repairing it locally after every insertion. Context: machines of the period
(e.g. Soviet BESM-class computers) had tiny core memories, so the extra space
had to be minimal — **two bits per node** suffice for the balance indicator.
**⚠ verify any claim about the specific machine; the paper itself does not
necessarily name one.**

### 1.3 Terminology and notation in the paper

- The paper speaks of a **tree** whose vertices hold records, each record with
  its key; left/right sub-trees hold smaller/larger keys.
- Each vertex carries a **balance indicator** taking three values (the height
  difference of its two sub-trees: −1, 0, +1). Our `Node::balance` reproduces
  exactly this; we use the sign convention *height(right) − height(left)*.
  **⚠ verify which sign convention the paper uses.**
- The paper does **not** use the word "rotation"; the restructuring is
  described as a re-arrangement of a few vertices. The name "AVL tree" and the
  "single/double rotation" vocabulary were introduced later (popularized by
  Knuth, TAOCP Vol. 3, §6.2.3). **⚠ verify.**

### 1.4 Curiosities

- It is generally regarded as the **first self-balancing binary search tree**,
  predating B-trees (Bayer & McCreight, 1972) and red–black trees
  (Bayer 1972; Guibas & Sedgewick 1978) by a decade.
- **Deletion is not covered** by the paper. Deletion with O(log N) rotations
  was worked out afterwards (C. C. Foster, "Information storage and retrieval
  using AVL trees", *Proc. ACM National Conference*, 1965, is an early
  Western treatment; Knuth 1973 discusses it). **⚠ verify.** Our `erase` is
  therefore an extension, clearly separated from the paper's algorithm.
- The worst-case trees are the **Fibonacci trees**: the minimum number of
  nodes in an AVL tree of height *h* is N(h) = F(h+2) − 1.

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

### 2.3 Deletion (extension, not in the paper)

Standard approach: delete as in a BST (replacing a two-child node by its
in-order successor), then retrace upward. Unlike insertion, a rotation can
*shorten* the subtree, so retracing may continue to the root: up to
O(log n) rotations per deletion. A rotation whose sibling has balance 0
preserves height and stops the retrace.

### 2.4 Ambiguities and our resolutions

| Issue | Paper | Our decision |
|---|---|---|
| Duplicate keys | **⚠ verify** (likely assumes distinct keys) | Overwrite value, no structural change |
| Retrace mechanism | **⚠ verify** (stack of the search path vs. back pointers) | Parent pointers (required by the course `Node`) |
| Balance-update formulas | Given as case tables for insertion | Generic formulas valid for any balances, so the same rotations serve deletion |
| Sign of balance | **⚠ verify** | `h(right) − h(left)` |
| Deletion | Absent | Implemented as an extension and labelled as such |

### 2.5 Implementation decisions introduced by C++

- `Node` is the course struct plus `std::int8_t balance` (the paper's
  three-valued indicator; 1 byte, padding makes the node 32 bytes on x86-64
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

_Filled in from `results/bench.csv`; see §3.3._

---

## 4. Legacy (5%)

- **Direct descendants in production code.**
  - OpenZFS / illumos: generic AVL tree library (`avl.h`) used throughout ZFS.
  - Windows NT kernel runtime: `RtlInsertElementGenericTableAvl` and related
    AVL "generic table" routines.
  - LLVM: `ImmutableSet`/`ImmutableMap` are built on a persistent AVL tree
    (`ImutAVLTree`).
  - OCaml standard library `Set`/`Map`: height-balanced trees in the AVL
    family (imbalance tolerance of 2 instead of 1).
  - GNU libavl (B. Pfaff): reference C implementations.
  **⚠ verify each against current source trees.**
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
   organization of information. *Soviet Math. Doklady* 3, 1259–1263.
   (Original: *Doklady AN SSSR* 146(2), 263–266.)
2. Knuth, D. E. (1998). *The Art of Computer Programming, Vol. 3: Sorting and
   Searching*, 2nd ed., §6.2.3 "Balanced Trees". Addison-Wesley.
3. Cormen, T. H. et al. (2022). *Introduction to Algorithms*, 4th ed. MIT Press
   (Problem 13-3, AVL trees).
4. Foster, C. C. (1965). Information storage and retrieval using AVL trees.
   *Proc. 20th ACM National Conference*, 192–205. **⚠ verify.**
5. Guibas, L. J., Sedgewick, R. (1978). A dichromatic framework for balanced
   trees. *FOCS 1978*, 8–21.
6. Bayer, R. (1972). Symmetric binary B-trees: data structure and maintenance
   algorithms. *Acta Informatica* 1, 290–306.
7. Haeupler, B., Sen, S., Tarjan, R. E. (2015). Rank-balanced trees.
   *ACM Transactions on Algorithms* 11(4), 30.
