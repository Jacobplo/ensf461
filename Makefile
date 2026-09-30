CC = gcc
OperatingSystem = $(shell uname -s)

ifeq ($(OperatingSystem),Linux)

.PHONY: all
all: gen_in_range.out gen_numbers.out select_arg.out

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
else
.PHONY: all
all:
	@echo "Sorry, I prefer Linux"

endif

.PHONY: clean
clean:
	rm -f *.out *.o

