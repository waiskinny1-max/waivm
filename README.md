# waivm

Register-based bytecode VM with a handwritten x86-64 assembly execution core.

## Features

- x86-64 NASM VM runtime for normal execution
- C17 assembler and disassembler
- custom `.waibc` bytecode format
- interactive debugger with stepping, register inspection, memory dumps, stack dumps, disassembly view, and breakpoints
- 8 signed 64-bit general-purpose registers
- 64 KiB bounds-checked linear memory
- stack operations and `call`/`ret`
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
; call/ret using the VM stack

mov r0, 21
call double
print r0
halt

double:
  mul r0, 2
  ret
```

```sh
./build/waivm run examples/call.wai
```

Expected output:

```text
42
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
./build/waivm asm examples/memory.wai -o memory.waibc
./build/waivm info memory.waibc
./build/waivm dis memory.waibc
./build/waivm run memory.waibc
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
| `load rX, [ADDR/rY]` | load signed 64-bit value from VM memory |
| `store [ADDR/rY], rX` | store signed 64-bit value into VM memory |
| `push rX` | push register value onto VM stack |
| `pop rX` | pop VM stack value into register |
| `cmp rX, IMM/rY` | compare and update zero flag |
| `jmp label` | unconditional jump |
| `jz rX, label` | jump when register is zero |
| `jnz rX, label` | jump when register is not zero |
| `je label` | jump when zero flag is set |
| `jne label` | jump when zero flag is clear |
| `call label` | push return address and jump |
| `ret` | pop return address and jump |
| `print rX` | print register value |
| `halt` | stop execution |

Registers are signed 64-bit values. Memory is 64 KiB, byte-addressed, and bounds-checked. `load` and `store` operate on 64-bit little-endian values.

## Bytecode Format

`.waibc` files use a 32-byte little-endian header followed by fixed-width 12-byte instructions.

Header:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 4 | magic bytes: `WAI0` |
| 4 | 2 | version: `3` |
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
| 4 | 8 | signed immediate / address / jump target |

## Architecture

The runtime boundary is deliberate:

- C owns CLI parsing, source loading, bytecode loading/writing, validation, assembler, disassembler, debugger UX, and tests.
- NASM owns the normal bytecode dispatch loop and executes VM instructions directly against the C-defined VM state.
- The debugger uses a C stepping engine so it can stop between instructions, inspect state, and resume predictably.
- C exposes a small print hook used by both execution paths.

See [`docs/architecture.md`](docs/architecture.md) and [`docs/vm-abi.md`](docs/vm-abi.md).

## Debugger

```sh
./build/waivm debug examples/call.wai
```

Useful commands:

```text
help
regs
ip
dis
mem 0 64
stack 8
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

Tests cover assembler parsing, VM execution, bytecode write/read, disassembly, debugger command flow, stack/call behavior, memory behavior, and basic error handling.

## Roadmap

See [`docs/roadmap.md`](docs/roadmap.md).

## Status

waivm is experimental but functional.

Current:

- register VM
- arithmetic and branch instructions
- bounds-checked linear memory
- stack operations
- `call`/`ret`
- handwritten NASM execution loop
- C17 source assembler
- `.waibc` bytecode writer/reader
- bytecode metadata `info` command
- disassembler
- interactive debugger
- example programs
- test harness and CI configuration

Not implemented yet:

- stack frames or local variable ABI
- heap allocation
- strings
- floating point values
- JIT compilation
- Windows runtime

## License

MIT
