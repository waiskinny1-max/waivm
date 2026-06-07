# Roadmap

## v0.1

- register-only VM;
- NASM execution loop;
- C assembler for `.wai` files;
- core arithmetic, branches, print, halt;
- basic examples and tests.

## v2

- `.waibc` binary file writer/reader;
- bytecode validation;
- `asm`, `dis`, `info`, and `debug` commands;
- C stepping engine for debugger;
- documentation and CI polish.

## v3

- 64 KiB VM memory;
- `load` and `store` instructions;
- VM stack with `push` and `pop`;
- `call` and `ret`;
- `cmp`, `je`, and `jne`;
- debugger memory dump;
- debugger stack dump;
- memory/call/compare examples and tests.

## Later Candidates

These are deliberately not implemented yet:

- stack-frame ABI;
- local variable conventions;
- function arguments/returns beyond raw registers and stack;
- static data section in `.waibc`;
- heap allocation;
- strings;
- floating point values;
- richer debugger commands;
- Windows x64 runtime;
- JIT compilation.
