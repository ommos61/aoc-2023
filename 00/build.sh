#!/usr/bin/env bash

set -e

CC="clang"
CFLAGS="-std=c99 -g -Wall -Wextra -Werror -pedantic"

function compile_file {
    src=$1
    target=$2

    [[ -f ${src} ]] && [[ ${src} -nt ${target} ]] && \
        echo "${src} -> ${target}" && ${CC} ${CFLAGS} -o ${target} ${src}
    return $?
}

day=$(basename ${PWD})
count=0
compile_file ./part1.c ./part1 && let count+=1
compile_file ./part2.c ./part2 && let count+=1
[[ $count != 0 ]] && echo "Day ${day}: compiled $count files"

exit 0
