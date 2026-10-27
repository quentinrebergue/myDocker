#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 7bis: Volume mounting"

# Create test directory
TEST_DIR="/tmp/mymoulette_vol_$$"
mkdir -p "$TEST_DIR"
echo "original" > "$TEST_DIR/file.txt"

echo "echo 'original' > $TEST_DIR/file.txt"
echo -n "Test volume accessible: "
if "$BIN" -v "$TEST_DIR" -I "$IMAGE" test -f /home/student/file.txt 2>&1; then
    echo "Volume accessible"
else
    echo " Volume not accessible"
    rm -rf "$TEST_DIR"
    exit 1
fi

echo -n "Test read from volume: "
CONTENT=$("$BIN" -v "$TEST_DIR" -I "$IMAGE" /bin/ash -c 'cat /home/student/file.txt' 2>&1)
if [ "$CONTENT" = "original" ]; then
    echo "Read: '$CONTENT'"
else
    echo " Wrong content: '$CONTENT'"
    rm -rf "$TEST_DIR"
    exit 1
fi

echo -n "Test write to volume: "
"$BIN" -v "$TEST_DIR" -I "$IMAGE" /bin/ash -c 'echo "modified" > /home/student/file.txt'
HOST_CONTENT=$(cat "$TEST_DIR/file.txt")
if [ "$HOST_CONTENT" = "modified" ]; then
    echo "Write successful: '$HOST_CONTENT'"
else
    echo " Write failed: '$HOST_CONTENT'"
    rm -rf "$TEST_DIR"
    exit 1
fi

rm -rf "$TEST_DIR"

exit 0
