#!/bin/sh

set -e

BIN=./out/mymoulette

echo "Palier 1: CGROUP"

# Run test_inside.sh inside the container with sudo
if "$BIN" tests/palier1/test_inside.sh; then
    exit 0
else
    exit 1
fi
