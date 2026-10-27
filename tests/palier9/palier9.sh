#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 9: pivot_root"

OUTPUT=$("$BIN" -I "$IMAGE" /bin/sh -c '
if [ -d /old_root ] || [ -d /.old_root ] || [ -d /oldroot ]; then
    echo "FOUND"
else
    echo "NOT_FOUND"
fi
')

if echo "$OUTPUT" | grep -q "NOT_FOUND"; then
    echo "Old root not accessible as expected"
else
    echo " Old root visible (not expected)"
    exit 1
fi

HOST_ROOT_INODE=$(stat -c %i / 2>/dev/null || stat -f %i / 2>/dev/null)
CONTAINER_ROOT_INODE=$("$BIN" -I "$IMAGE" stat -c %i / 2>/dev/null || echo "unknown")

if [ "$HOST_ROOT_INODE" = "$CONTAINER_ROOT_INODE" ]; then
    echo " Root inode are the same : Host='$HOST_ROOT_INODE' Container='$CONTAINER_ROOT_INODE'"
    exit 1
fi
echo "Root inode different as expected : Host=$HOST_ROOT_INODE Container=$CONTAINER_ROOT_INODE"

exit 0
