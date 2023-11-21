#!/usr/bin/env bash

set -e

echo "Compiling all days..."
for d in [012][0-9]
do
    echo "Day ${d}"
    pushd ${d} >/dev/null 2>&1
    ./build.sh
    popd >/dev/null 2>&1
done
echo "Done"

exit 0
