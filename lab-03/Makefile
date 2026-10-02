CC := gcc
CFLAGS := -Wall


.DEFAULT_GOAL := all
.PHONY: all
all: shell.out

.PHONY: clean
clean:
	rm -rf *.out *.o

.PHONY: run
run: shell.out
	@./shell.out

.PHONY: test
test: test.out
	@./test.out

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

shell.out: shell.o parser.o
	$(CC) -o $@ $^

test.out: test.o parser.o
	$(CC) -o $@ $^
