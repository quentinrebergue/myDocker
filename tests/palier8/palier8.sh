#!/bin/sh

BIN=./out/mymoulette
IMAGE=library/alpine:latest

echo "Palier 8: Container identification"

HOSTNAME1=$("$BIN" -I "$IMAGE" hostname)
HOSTNAME2=$("$BIN" -I "$IMAGE" hostname)

echo "Container 1: $HOSTNAME1"
echo "Container 2: $HOSTNAME2"

if [ -z "$HOSTNAME1" ]; then
    echo " Hostname is empty"
    exit 1
fi

if [ "$HOSTNAME1" != "$HOSTNAME2" ]; then
    echo "Hostnames are unique"
else
    echo " Hostnames are identical"
    exit 1
fi

exit 0
