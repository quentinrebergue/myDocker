#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest
TEST_DIR=tests/palier4

echo "Palier 4: Seccomp"

echo "Compiling test program on host..."
gcc -static -o "$TEST_DIR/test_syscalls" "$TEST_DIR/test_syscalls.c"

OUTPUT=$("$BIN" "$TEST_DIR/test_syscalls")

echo "$OUTPUT"

if echo "$OUTPUT" | grep -q "Test NOK"; then
    exit 1
elif echo "$OUTPUT" | grep -q "Test OK"; then
    exit 0
else
    echo "Unexpected output"
    exit 1
fi
