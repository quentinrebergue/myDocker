#!/bin/sh

if [ -e $(which ping) ]; then
    OUTPUT=$(ping -c 1 -W 1 0.0.0.0)
    
    if echo "$OUTPUT" | grep -qi "permission denied"; then
        echo "CAP_NET_RAW missing"
        exit 1
    else
        echo "CAP_NET_RAW present"
    fi
else
    echo "ping command not found, skipping this test"
fi

if [ -e $(which date) ]; then
    OUTPUT=$(date -s "2026-01-01" 2>&1)
    
    if echo "$OUTPUT" | grep -qi "permission denied\|operation not permitted"; then
        echo "CAP_SYS_TIME is missing, as expected"
    else
        echo "CAP_SYS_TIME is present, which is unexpected"
        exit 1
    fi
else
    echo "date command not found, skipping this test"
fi

if [ -e $(which mount) ]; then
    OUTPUT=$(mount -t tmpfs none /mnt 2>&1)
    
    if echo "$OUTPUT" | grep -qi "permission denied\|operation not permitted"; then
        echo "CAP_SYS_ADMIN is missing, as expected"
    else
        echo "CAP_SYS_ADMIN is present, which is unexpected"
        exit 1
    fi
else
    echo "mount command not found, skipping this test"
fi

exit 0
