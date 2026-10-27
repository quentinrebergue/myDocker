#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 6: Namespace"

HOST_HOSTNAME=$(hostname)
HOST_MNT_NS=$(readlink /proc/self/ns/mnt 2>/dev/null)

CONTAINER_HOSTNAME=$("$BIN" /bin/bash -c hostname)
if [ "$CONTAINER_HOSTNAME" != "$HOST_HOSTNAME" ]; then
    echo "Hostname isolated: host='$HOST_HOSTNAME', container='$CONTAINER_HOSTNAME'"
else
    exit 1
fi

# Get container mount namespace
CONTAINER_MNT_NS=$("$BIN" /bin/sh -c "readlink /proc/self/ns/mnt")
if [ "$CONTAINER_MNT_NS" != "$HOST_MNT_NS" ]; then
    echo "Mount namespace isolated: host='$HOST_MNT_NS', container='$CONTAINER_MNT_NS'"
else
    echo "Mount namespace not isolated: both='$HOST_MNT_NS'"
    exit 1
fi

OUTPUT=$("$BIN" tests/palier6/test_inside.sh 2>&1)
EXIT_CODE=$?

echo "$OUTPUT"
if [ $EXIT_CODE -eq 0 ]; then
    exit 0
else
    exit 1
fi
