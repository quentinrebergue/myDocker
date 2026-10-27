#!/bin/sh

CGROUP_PATH="/sys/fs/cgroup/mymoulette"

if [ -f /proc/self/cgroup ]; then
    CURRENT_CGROUP=$(cat /proc/self/cgroup)
    echo "Current cgroup: $CURRENT_CGROUP"
    if echo "$CURRENT_CGROUP" | grep -q "mymoulette"; then
        echo "Process is in mymoulette cgroup"
    else
        echo "Process is in wrong cgroup: $CURRENT_CGROUP"
        exit 1
    fi
fi

if [ -e "${CGROUP_PATH}/memory.max" ]; then
    MEM_MAX=$(cat "${CGROUP_PATH}/memory.max")
    # Accept either 1GB (1000000000) or 1GiB (1073741824)
    if [ "$MEM_MAX" -ge 999000000 ] && [ "$MEM_MAX" -le 1100000000 ]; then
        echo "Memory limit is set correctly"
    else
        echo "Memory limit is wrong: $MEM_MAX (expected 1GB)"
        exit 1
    fi
else
    echo "Memory limit not found"
    exit 1
fi

if [ -e "${CGROUP_PATH}/cpu.max" ]; then
    CPU_MAX=$(cat "${CGROUP_PATH}/cpu.max")
    echo "CPU limit configured: $CPU_MAX"
else
    echo "CPU limit not found"
    exit 1
fi

if [ -e "${CGROUP_PATH}/pids.max" ]; then
    PID_MAX=$(cat "${CGROUP_PATH}/pids.max")
    if [ "$PID_MAX" -eq "100" ]; then
        echo "PID limit is set at 100"
    else
        echo "PID limit is wrong: $PID_MAX"
        exit 1
    fi
else
    echo "PID limit not found"
    exit 1
fi

exit 0
