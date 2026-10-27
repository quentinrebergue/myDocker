#!/bin/sh

FAILED=0

echo -n "Current PID: "
PID=$$
echo $PID
if [ "$PID" != "1" ]; then
    echo "PID namespace not isolated: expected 1, got $PID"
    FAILED=1
else
    echo "PID namespace isolated"
fi

if ip a 2>/dev/null | grep -q "lo"; then
    echo "network namespace present"
else
    echo "network namespace absent"
    FAILED=1
fi
echo

echo "[IPC namespace]"
ipcs 2>/dev/null || echo "=> OK: ipcs isolated or unavailable"
echo


if [ $FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi
