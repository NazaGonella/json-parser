#!/bin/bash

mkdir -p ./build
rm -rf ./build/*

pushd ./build > /dev/null

    sources=(../*.c)
    gcc -g -Wall -Wextra -pedantic "${sources[@]}" -o main
    
popd > /dev/null