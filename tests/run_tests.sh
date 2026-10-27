#!/bin/sh


BIN=./out/mymoulette
IMAGE=library/alpine:latest

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

passed=0
failed=0

run_test() {
    local palier=$1
    local test_script="tests/palier${palier}/palier${palier}.sh"
    
    if [ ! -f "$test_script" ]; then
        echo -e "Palier $palier: NOT FOUND !!!"
        return
    fi
    if bash "$test_script"; then
        echo -e "${GREEN}Palier $palier: PASSED${NC}"
        ((passed++))
    else
        echo -e "${RED}Palier $palier: FAILED${NC}"
        ((failed++))
    fi
    echo ""
}

echo "========================================"
echo "   MYMOULETTE TEST SUITE"
echo "========================================"

for palier in 0 1 2 3 4 5 6 7 7bis 8 9; do
    run_test "$palier"
done

echo ""
echo "========================================"
echo "   RESULTS"
echo "========================================"
echo -e "${GREEN}Passed: $passed${NC}"
echo -e "${RED}Failed: $failed${NC}"
echo "========================================"

[ $failed -eq 0 ]
