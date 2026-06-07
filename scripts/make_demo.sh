#!/usr/bin/env sh
set -eu
cmake -S . -B build
cmake --build build
./build/waivm run examples/sum.wai
./build/waivm run examples/factorial.wai
