# waivm

Register-based bytecode VM with a handwritten x86-64 assembly execution core.

## Features

Current v0.1 scope:

- x86-64 NASM VM runtime
- C17 assembler for `.wai` source files
- fixed-width internal bytecode instruction encoding
- 8 signed 64-bit VM registers: `r0` through `r7`
- arithmetic, branch, print, and halt instructions
- examples and CTest-based smoke tests

Planned later:

- persistent `.waibc` bytecode files
- bytecode disassembler
- interactive debugger
- bytecode metadata inspection
- broader test coverage and CI release artifacts

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

## CLI

Implemented in v0.1:

```sh
waivm run <file.wai>
waivm help
```

Reserved for later versions:

```sh
waivm asm <input.wai> -o <output.waibc>
waivm run <file.waibc>
waivm dis <file.waibc>
waivm debug <file.wai|file.waibc>
waivm info <file.waibc>
```

The CLI rejects unimplemented commands instead of pretending they exist.

## Instruction Set

v0.1 accepts this source-level instruction set:

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

Registers are signed 64-bit values. The runtime rejects division by zero and invalid bytecode state.

## Bytecode Format

v0.1 uses a fixed-width in-memory instruction encoding:

```c
typedef struct wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    int64_t imm;
} wai_instruction;
```

The struct is packed to 12 bytes. Persistent `.waibc` files are intentionally deferred to v0.3.

## Architecture

The runtime boundary is deliberate:

- C owns parsing, source loading, validation, VM initialization, and CLI behavior.
- NASM owns the bytecode dispatch loop and executes VM instructions directly against the C-defined VM state.
- C exposes a tiny print hook used by the assembly runtime.

See [`docs/architecture.md`](docs/architecture.md) and [`docs/vm-abi.md`](docs/vm-abi.md).

## Debugger

Not implemented in v0.1. The planned debugger is documented in [`docs/debugger.md`](docs/debugger.md).

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
ctest --test-dir build
```

The v0.1 tests cover example execution, assembler parsing, runtime error handling, and basic validation.

## Roadmap

See [`docs/roadmap.md`](docs/roadmap.md).

## Status

waivm is experimental but functional.

Current:

- register VM
- arithmetic and branch instructions
- handwritten NASM execution loop
- C17 source assembler
- example programs
- test harness

Not implemented yet:

- persistent `.waibc` files
- disassembler
- interactive debugger
- metadata `info` command
- stack frames
- function calls
- heap allocation
- JIT compilation
- Windows runtime

## License

MIT
