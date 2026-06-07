# waivm

Register-based bytecode VM with a handwritten x86-64 assembly execution core.

## Features

- x86-64 NASM VM runtime for normal execution
- C17 assembler and disassembler
- custom `.waibc` bytecode format
- interactive debugger with stepping, register inspection, disassembly view, and breakpoints
- 8-register signed integer VM
- fixed-width 12-byte instruction encoding
- examples, tests, and GitHub Actions CI

## Quickstart

```sh
cmake -S . -B build
cmake --build build
./build/waivm run examples/sum.wai
```

Expected output:

```text
55
```

## Example Program

```asm
; factorial of 5

mov r0, 5      ; counter
mov r1, 1      ; result

loop:
  mul r1, r0
  sub r0, 1
  jnz r0, loop

print r1
halt
```

```sh
./build/waivm run examples/factorial.wai
```

Expected output:

```text
120
```

## CLI

```sh
waivm run <file.wai|file.waibc>
waivm asm <input.wai> -o <output.waibc>
waivm dis <file.wai|file.waibc>
waivm debug <file.wai|file.waibc>
waivm info <file.waibc>
waivm help
```

Example bytecode workflow:

```sh
./build/waivm asm examples/sum.wai -o sum.waibc
./build/waivm info sum.waibc
./build/waivm dis sum.waibc
./build/waivm run sum.waibc
```

## Instruction Set

| Instruction | Meaning |
|---|---|
| `mov rX, IMM` | store immediate in register |
| `mov rX, rY` | copy register |
| `add rX, IMM/rY` | add into register |
| `sub rX, IMM/rY` | subtract from register |
| `mul rX, IMM/rY` | multiply register |
| `div rX, IMM/rY` | signed integer division |
| `jmp label` | unconditional jump |
| `jz rX, label` | jump when register is zero |
| `jnz rX, label` | jump when register is not zero |
| `print rX` | print register value |
| `halt` | stop execution |

Registers are signed 64-bit values. Division by zero, invalid registers, bad opcodes, and invalid jumps fail cleanly.

## Bytecode Format

`.waibc` files use a 32-byte little-endian header followed by fixed-width 12-byte instructions.

Header:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 4 | magic bytes: `WAI0` |
| 4 | 2 | version: `1` |
| 6 | 2 | header size: `32` |
| 8 | 2 | instruction size: `12` |
| 10 | 2 | register count: `8` |
| 12 | 4 | flags: `0` |
| 16 | 8 | code instruction count |
| 24 | 8 | data size: `0` |

Instruction:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | opcode |
| 1 | 1 | operand `a` |
| 2 | 1 | operand `b` |
| 3 | 1 | reserved operand `c` |
| 4 | 8 | signed immediate / jump target |

## Architecture

The runtime boundary is deliberate:

- C owns CLI parsing, source loading, bytecode loading/writing, validation, assembler, disassembler, debugger UX, and tests.
- NASM owns the normal bytecode dispatch loop and executes VM instructions directly against the C-defined VM state.
- The debugger uses a C stepping engine so it can stop between instructions, inspect state, and resume predictably.
- C exposes a small print hook used by both execution paths.

See [`docs/architecture.md`](docs/architecture.md) and [`docs/vm-abi.md`](docs/vm-abi.md).

## Debugger

```sh
./build/waivm debug examples/sum.wai
```

Useful commands:

```text
help
regs
ip
dis
step
break 5
continue
clear 5
quit
```

## Build

Dependencies:

- CMake 3.20+
- C17 compiler
- NASM
- Make or Ninja
- Linux x86-64

```sh
cmake -S . -B build
cmake --build build
```

## Tests

```sh
ctest --test-dir build --output-on-failure
```

Tests cover assembler parsing, VM execution, bytecode write/read, disassembly, debugger command flow, and basic error handling.

## Roadmap

See [`docs/roadmap.md`](docs/roadmap.md).

## Status

waivm is experimental but functional.

Current:

- register VM
- arithmetic and branch instructions
- handwritten NASM execution loop
- C17 source assembler
- `.waibc` bytecode writer/reader
- bytecode metadata `info` command
- disassembler
- interactive debugger
- example programs
- test harness and CI configuration

Not implemented yet:

- stack frames
- function calls
- heap allocation
- strings
- floating point values
- JIT compilation
- Windows runtime

## License

MIT
