#!/usr/bin/env python3
"""Plots results/bench.csv. Optional: requires matplotlib.
Usage: plot.py bench.csv out_dir"""
import csv, math, sys, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

rows = list(csv.DictReader(open(sys.argv[1])))
out = sys.argv[2]
os.makedirs(out, exist_ok=True)

def series(struct, pattern, col):
    r = [x for x in rows if x["structure"] == struct and x["pattern"] == pattern]
    return [int(x["n"]) for x in r], [float(x[col]) for x in r]

# 1. Height vs n, with the AVL worst-case bound and log2 n.
fig, ax = plt.subplots()
for s, p in [("avl", "random"), ("avl", "ascending"), ("naive_bst", "random")]:
    n, h = series(s, p, "height")
    ax.plot(n, h, marker="o", label=f"{s} / {p}")
n, _ = series("avl", "random", "height")
ax.plot(n, [1.4405 * math.log2(k + 2) - 0.3277 for k in n], "k--", label="AVL bound 1.4405 log2(n+2) - 0.3277")
ax.plot(n, [math.log2(k + 1) for k in n], "k:", label="log2(n+1) (perfect)")
ax.set_xscale("log", base=2); ax.set_xlabel("n"); ax.set_ylabel("height"); ax.legend(); ax.grid(alpha=.3)
fig.savefig(f"{out}/height.png", dpi=150, bbox_inches="tight")

# 2. Time per operation.
for col, fname in [("insert_ns", "insert_time.png"), ("find_hit_ns", "find_time.png")]:
    fig, ax = plt.subplots()
    for s in ["avl", "std_map", "naive_bst"]:
        for p in ["random", "ascending"]:
            n, t = series(s, p, col)
            if n:
                ax.plot(n, t, marker="o", label=f"{s} / {p}")
    ax.set_xscale("log", base=2); ax.set_yscale("log"); ax.set_xlabel("n"); ax.set_ylabel(f"{col} (ns/op)")
    ax.legend(); ax.grid(alpha=.3)
    fig.savefig(f"{out}/{fname}", dpi=150, bbox_inches="tight")

# 3. Comparisons per successful search vs log2 n.
fig, ax = plt.subplots()
for s, p in [("avl", "random"), ("avl", "ascending"), ("naive_bst", "random")]:
    n, c = series(s, p, "cmp_per_hit")
    ax.plot(n, c, marker="o", label=f"{s} / {p}")
ax.plot(n, [math.log2(k) for k in n], "k:", label="log2 n")
ax.set_xscale("log", base=2); ax.set_xlabel("n"); ax.set_ylabel("comparisons / successful find"); ax.legend(); ax.grid(alpha=.3)
fig.savefig(f"{out}/comparisons.png", dpi=150, bbox_inches="tight")
print("plots written to", out)
