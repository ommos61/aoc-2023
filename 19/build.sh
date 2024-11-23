#!/usr/bin/env bash

set -e

CC="clang"
CFLAGS="-std=c99 -g -Wall -Wextra -Werror -pedantic"

errors=0
compiles=0
function compile_file {
    src=$1
    target=$2

    if [[ -f ${src} ]] && [[ ${src} -nt ${target} ]]
    then
        echo "${src} -> ${target}"
        ${CC} ${CFLAGS} -o ${target} ${src}
        let compiles+=1
        status=$?
        if [[ $status != 0 ]]
        then
            let errors+=1
        fi
    fi
    return $status
}

day=$(basename ${PWD})
compile_file ./part1.c ./part1
compile_file ./part2.c ./part2
[[ $compiles != 0 ]] && echo "Day ${day}: compiled $compiles files"

[[ $errors != 0 ]] && exit 1
exit 0
