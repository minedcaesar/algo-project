#!/usr/bin/env bash
# Builds and runs the benchmark, writing results/bench.csv and results/env.txt.
# Usage: scripts/run_bench.sh [max_log2_n=20] [reps=5] [seed=12345]
set -euo pipefail
cd "$(dirname "$0")/.."
make -s build/bench_avl
mkdir -p results
{
  echo "date:     $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "compiler: $(${CXX:-g++} --version | head -1)"
  echo "flags:    -std=c++20 -O2 -Wall -Wextra -Werror"
  echo "kernel:   $(uname -sr)"
  echo "cpu:      $(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2- | sed 's/^ //' || echo unknown)"
  echo "cores:    $(nproc 2>/dev/null || echo unknown)"
  echo "args:     max_log2_n=${1:-20} reps=${2:-5} seed=${3:-12345}"
} > results/env.txt
./build/bench_avl "${1:-20}" "${2:-5}" "${3:-12345}" | tee results/bench.csv
if python3 -c "import matplotlib" 2>/dev/null; then
  python3 scripts/plot.py results/bench.csv results
else
  echo "(matplotlib not found: skipping plots; CSV is in results/bench.csv)" >&2
fi
