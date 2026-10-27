#!/bin/sh

BIN=./out/mymoulette

echo "Palier 2: Capabilities"

# Run tests with apline image because ping could work
OUTPUT=$("$BIN" -v tests/palier2/ -I library/alpine:latest /home/student/test_inside.sh)
EXIT_CODE=$?

echo "$OUTPUT"

if [ $EXIT_CODE -eq 0 ]; then
    exit 0
else
    exit 1
fi
