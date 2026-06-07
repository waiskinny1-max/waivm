#!/usr/bin/env sh
set -eu

cmake -S . -B build
cmake --build build

./build/waivm run examples/sum.wai
./build/waivm run examples/factorial.wai
./build/waivm run examples/memory.wai
./build/waivm run examples/call.wai
./build/waivm run examples/bitwise.wai
./build/waivm asm examples/bitwise.wai -o bitwise.waibc
./build/waivm info bitwise.waibc
./build/waivm dis bitwise.waibc
