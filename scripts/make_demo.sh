#!/usr/bin/env sh
set -eu
cmake -S . -B build
cmake --build build
./build/waivm run examples/sum.wai
./build/waivm asm examples/sum.wai -o sum.waibc
./build/waivm info sum.waibc
./build/waivm dis sum.waibc
./build/waivm run sum.waibc
