# kolosalne: a calculator for colossal numbers in C

A stack calculator for numbers far too large for any positional notation. It
can add and multiply values such as 2^(2^(2^65536)), whose binary expansion
would need more bits than there are atoms on Earth, while storing them in a
handful of bytes.

The project was built for an introductory programming course at the University
of Warsaw. Identifiers and comments in the code are in Polish, as the course
requires.

## The representation

A *colossal number* is a list of digits, and each digit is itself a colossal
number. Its value is the sum of powers of two whose exponents are the values of
its digits. The representation was studied, in an equivalent form, by Donald
Knuth.

In text form a number is written as `{ "1" <digit> } "0"`, which is a balanced
bracket expression with `1` as the opening bracket and `0` as the closing one:

| Value | Text form |
| --- | --- |
| 0 | `0` |
| 1 | `100` |
| 2 | `11000` |
| 10 | `1110010011000` |
| 65536 | `11111000000` |
| 2^65536 | `1111110000000` |
| 2^(2^65536) | `111111100000000` |

A number is *normalized* when its digits are normalized and strictly
decreasing. Every non-negative integer has exactly one normalized form, and the
calculator always prints that form.

## Commands

The program reads one command per line from standard input:

| Command | Effect |
| --- | --- |
| a colossal number | push it (it does not have to be normalized) |
| `.` | pop a number and print it in normalized form |
| `:` | duplicate the top of the stack |
| `^` | pop `x`, push 2^x |
| `+` | pop two numbers, push their sum |
| `*` | pop two numbers, push their product |

```
$ printf '1110010011000\n:\n*\n.\n' | ./kolosalne
11110001100011110001001110000
```

This squares 10 and prints 100.

## How it works

- **Recursive data structure.** A number is a singly linked list of terms
  2^exponent, ordered from the largest exponent, where each exponent points to
  another number. `NULL` stands for zero.
- **Immutable nodes with reference counting.** Numbers are never modified, so
  sub-structures are shared freely: duplicating a number or raising 2 to it
  costs O(1) and copies nothing. Each node carries a reference count and is
  freed as soon as the last reference disappears.
- **Comparison** walks two normalized numbers term by term, starting from the
  largest exponents, and compares the exponents recursively.
- **Addition** merges the two term lists. When both contain the same exponent
  E, the two terms become a single 2^(E+1), and the carry propagates upwards
  through further merging.
- **Normalization on input** falls out of addition: the parser adds the terms
  one by one, so any valid input, however unordered or repetitive, ends up
  normalized.
- **Multiplication** distributes one number over the terms of the other, using
  2^a · 2^b = 2^(a+b), and adds the partial products.

No fixed limits are placed on the size of the input, the numbers or the stack;
everything lives on the heap and the stack grows by doubling.

## Building and running

Requires GCC 14 or newer (for `-std=c23`) and `make`.

```bash
make                # or: make CC=gcc-14
make test           # compares tests/*.in against tests/*.out

./kolosalne < tests/mnozenie.in
```

The program is compiled with the strict flag set required by the course (see
[`opcje`](opcje)): `-Wall -Wextra -Wconversion -Wshadow -Werror` and more, with
the undefined-behaviour sanitizer enabled.

## Tests

`make test` runs every input in [`tests/`](tests) and compares the result with
the expected output. When Valgrind is installed the program runs under
`valgrind --leak-check=full --error-exitcode=1`, so any memory error or leak
fails the test. The same checks run on every push through GitHub Actions.

The test files cover normalization of several different spellings of the
number ten, addition with carries, multiplication, and towers of powers up to
5 + 2^(2^(2^65536)).
