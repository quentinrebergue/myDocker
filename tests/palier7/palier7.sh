#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 7: Mount"

echo -n "Host mounts:"
HOST_MOUNTS=$(mount | wc -l)
echo "  $HOST_MOUNTS mount points"

echo -n "Container mounts:"
CONTAINER_MOUNTS=$("$BIN" -I "$IMAGE" /bin/sh -c 'mount 2>/dev/null | wc -l' || echo "0")
echo "  $CONTAINER_MOUNTS mount points"

if [ "$CONTAINER_MOUNTS" -lt "$HOST_MOUNTS" ] && [ "$CONTAINER_MOUNTS" -gt 0 ]; then
    echo "Container has fewer mounts (isolated)"
else
    echo " Mount isolation unclear"
    exit 1
fi

echo -n "/proc isolation:"
PROC_COUNT=$("$BIN" -I "$IMAGE" /bin/sh -c 'ls /proc/[0-9]* -d 2>/dev/null | wc -l')
echo "  $PROC_COUNT processes visible"
if [ "$PROC_COUNT" -lt 50 ]; then
    echo "/proc appears isolated"
else
    echo " Many processes visible"
    exit 1
fi

exit 0
