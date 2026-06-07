#!/usr/bin/env sh
set -eu

cmake -S . -B build
cmake --build build

./build/waivm run examples/sum.wai
./build/waivm run examples/factorial.wai
./build/waivm run examples/memory.wai
./build/waivm run examples/call.wai
./build/waivm asm examples/call.wai -o call.waibc
./build/waivm info call.waibc
./build/waivm dis call.waibc
