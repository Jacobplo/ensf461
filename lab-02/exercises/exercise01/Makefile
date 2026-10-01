CC = gcc
OS = $(shell uname -s)

.PHONY: all
all: check_linux gen_in_range.out gen_numbers.out select_arg.out

%.o: %.c
	$(CC) -c -o $@ $<

gen_in_range.out: gen_in_range.o
	$(CC) -o $@ $<

gen_numbers.out: gen_numbers.o utils.o
	$(CC) -o $@ $^

select_arg.out: select_arg.o utils.o
	$(CC) -o $@ $^

.PHONY: test
test: gen_in_range.out gen_numbers.out select_arg.out
	./gen_in_range.out 1 10
	./gen_numbers.out 5
	./select_arg.out 3 1 2 3 4 5

.PHONY: check_linux
check_linux:
	@if [ $(OS) != "Linux" ]; then \
		echo "Sorry, I prefer Linux" >&2; \
		false; \
	fi

.PHONY: clean
clean:
	rm -f *.out *.o

