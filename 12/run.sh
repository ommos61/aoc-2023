#!/usr/bin/env bash

set -e

if [ -f input_sample.txt ]
then
    [ -f ./part1 ] && echo "---------->Running part1 sample..."
    [ -f ./part1 ] && ./part1 input_sample.txt
    [ -f ./part2 ] && echo "----------> Running part2 sample..."
    [ -f ./part2 ] && ./part2 input_sample.txt
fi

[ -f ./part1 ] && echo "---------->Running part1..."
[ -f ./part1 ] && ./part1
[ -f ./part2 ] && echo "---------->Running part2..."
[ -f ./part2 ] && ./part2

exit 0
