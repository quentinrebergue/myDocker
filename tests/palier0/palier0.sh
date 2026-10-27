#!/bin/sh

echo "Palier 0: Testing project structure"

if [ ! -f "Makefile" ]; then
    echo "Makefile missing"
    exit 1
fi
echo "Makefile found"

if [ ! -f "Dockerfile" ]; then
    echo "Dockerfile missing"
    exit 1
fi
echo "Dockerfile found"

if [ ! -d "src" ]; then
    echo "src/ directory missing"
    exit 1
fi
echo "src/ found"

if [ ! -f "out/mymoulette" ]; then
    echo "Binary not found in out/"
    exit 1
fi
echo "Binary found in out/"

if file out/mymoulette | grep -q "statically linked"; then
    echo "Binary is statically linked"
else
    echo "Binary may not be statically linked"
fi

if [ ! -x "out/mymoulette" ]; then
    echo "Binary is not executable"
    exit 1
fi
echo "Binary is executable"

exit 0
