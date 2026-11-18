CXX      ?= g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Werror -Iinclude
ASAN     := -fsanitize=address,undefined -fno-omit-frame-pointer -g
BUILD    := build

SRC      := src/avl_tree.cpp
HDR      := include/avl_tree.hpp include/naive_bst.hpp include/metrics.hpp

.PHONY: all test asan bench clean

all: $(BUILD)/test_avl $(BUILD)/bench_avl

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test_avl: tests/test_avl.cpp $(SRC) $(HDR) | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/test_avl.cpp $(SRC) -o $@

$(BUILD)/test_avl_asan: tests/test_avl.cpp $(SRC) $(HDR) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(ASAN) tests/test_avl.cpp $(SRC) -o $@

$(BUILD)/bench_avl: bench/bench_avl.cpp $(SRC) $(HDR) | $(BUILD)
	$(CXX) $(CXXFLAGS) bench/bench_avl.cpp $(SRC) -o $@

test: $(BUILD)/test_avl
	./$(BUILD)/test_avl

# AddressSanitizer (+UBSan): any leak, use-after-free or UB fails the run.
asan: $(BUILD)/test_avl_asan
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ./$(BUILD)/test_avl_asan

bench: $(BUILD)/bench_avl
	./scripts/run_bench.sh

clean:
	rm -rf $(BUILD)
