#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 3: chroot isolation"

HOST_FIREFOX=$(which firefox)

CONTAINER_OUTPUT=$("$BIN" -I "$IMAGE" /bin/ash -c 'which firefox || echo NOT_FOUND')
if [ -n "$HOST_FIREFOX" ]; then
    if echo "$CONTAINER_OUTPUT" | grep -qi "NOT_FOUND"; then
        echo "firefox found in host and not found in container (isolated)"
    else
        echo "firefox found in container too: $CONTAINER_OUTPUT"
        echo "Not properly isolated"
        exit 1
    fi
else
    echo "Host dont have firefox, skipping firefox isolation test"
fi

exit 0
