# Use a GCC that supports -std=c23 (GCC 14+), e.g. `make CC=gcc-14`.
CC = gcc

kolosalne: kolosalne.c opcje
	$(CC) @opcje kolosalne.c -o kolosalne

test: kolosalne
	./test.sh

clean:
	rm -f kolosalne

.PHONY: test clean
