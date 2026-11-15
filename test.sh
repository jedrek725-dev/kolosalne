#!/bin/sh
# Runs every tests/*.in through the calculator and compares the output with
# the matching .out file. Uses Valgrind (as the course graders do) when it is
# installed, so memory errors and leaks also fail the test.
set -u
fail=0

if command -v valgrind > /dev/null 2>&1; then
    RUN="valgrind --leak-check=full -q --error-exitcode=1 ./kolosalne"
else
    RUN="./kolosalne"
    echo "valgrind not found, running without memory checks"
fi

for in in tests/*.in; do
    out="${in%.in}.out"
    if $RUN < "$in" | diff - "$out" > /dev/null; then
        echo "ok   $in"
    else
        echo "FAIL $in"
        fail=1
    fi
done

exit $fail
