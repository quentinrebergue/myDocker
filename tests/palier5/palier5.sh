#!/bin/sh

BIN=./out/mymoulette

echo "Palier 5: OCI image"

OUTPUT=$("$BIN" -I library/alpine:latest /bin/ash -c 'cat /etc/alpine-release || echo FAILED')
if echo "$OUTPUT" | grep -q "FAILED"; then
    echo " alpine:latest failed"
    exit 1
fi
echo "alpine:latest => $OUTPUT"

OUTPUT=$("$BIN" -I library/alpine:3.18 /bin/ash -c 'cat /etc/alpine-release || echo FAILED')
if echo "$OUTPUT" | grep -q "FAILED"; then
    echo " alpine:3.18 not available"
else
    echo "alpine:3.18 => $OUTPUT"
    if [[ ! "$OUTPUT" =~ ^3\.18\..* ]]; then
        echo " Unexpected alpine:3.18 version"
        exit 1
    fi
fi

OUTPUT=$("$BIN" -I library/python:3.11-slim python3 --version 2>&1 || echo "FAILED")
if echo "$OUTPUT" | grep -q "FAILED"; then
    echo " python:3.11-slim not available"
else
    echo "python:3.11-slim => $OUTPUT"
    if ! echo "$OUTPUT" | grep -q "Python 3.11"; then
        echo " Unexpected Python version"
        exit 1
    fi
fi

OUTPUT=$("$BIN" -I library/nginx:alpine nginx -v 2>&1 || echo "FAILED")
if echo "$OUTPUT" | grep -q "FAILED"; then
    echo " nginx:alpine not available"
else
    echo "nginx:alpine => $OUTPUT"
    if ! echo "$OUTPUT" | grep -q "nginx"; then
        echo " Unexpected nginx output"
        exit 1
    fi
fi

exit 0
