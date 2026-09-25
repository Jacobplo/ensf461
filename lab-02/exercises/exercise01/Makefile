OS := $(shell uname)

CC := gcc

BUILD_DIR := build

TARGETS := $(BUILD_DIR)/gen_in_range.out \
           $(BUILD_DIR)/gen_numbers.out \
           $(BUILD_DIR)/select_arg.out

COMMON := utils.c

# =============
# Basic targets
# =============

.DEFAULT_GOAL := all
.PHONY: all
all: check_linux $(TARGETS) test

$(BUILD_DIR)/%.out: %.c $(COMMON)
	@mkdir -p $(BUILD_DIR)
	@printf "\tCC\t$@\n"
	@$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# =====
# Tests
# =====

.PHONY: check_linux
check_linux:
	@if [ $(OS) != "Linux" ]; then \
		echo "Sorry, I prefer Linux" >&2; \
		false; \
	fi

.PHONY: test
test: test_gen_in_range test_gen_numbers test_select_arg

.PHONY: test_gen_in_range
test_gen_in_range: $(BUILD_DIR)/gen_in_range.out
	@printf "\tTEST\t$<\n"
	@./build/gen_in_range.out 0 5 >/dev/null

.PHONY: test_gen_numbers
test_gen_numbers: $(BUILD_DIR)/gen_numbers.out
	@printf "\tTEST\t$<\n"
	@./build/gen_numbers.out 5 >/dev/null

.PHONY: test_select_arg
test_select_arg: $(BUILD_DIR)/select_arg.out
	@printf "\tTEST\t$<\n"
	@./build/select_arg.out 1 2 3 4 5 >/dev/null
