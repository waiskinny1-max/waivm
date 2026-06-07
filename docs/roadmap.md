# Roadmap

## v0.1

Implemented:

- register VM;
- arithmetic and branch instructions;
- `.wai` source assembler;
- NASM execution loop;
- basic examples and tests.

## v2

Implemented:

- `.waibc` bytecode writer/reader;
- `asm`, `dis`, `info`, and `debug` commands;
- bytecode validation;
- documentation and CI polish.

## v3

Implemented:

- `load` and `store` instructions;
- VM stack with `push` and `pop`;
- `call` and `ret`;
- `cmp`, `je`, and `jne`;
- debugger memory and stack dumps;
- memory/call/compare examples and tests.

## v4

Implemented:

- `nop`;
- signed integer remainder via `mod`;
- bitwise `and`, `or`, `xor`, and `not`;
- logical `shl` and `shr`;
- shift-count validation;
- bitwise/modulo examples and tests;
- updated bytecode version and documentation.

## v5

Implemented:

- `verify` command for source and bytecode programs;
- structural bytecode verifier with detailed error reports;
- `trace` command for instruction-level execution traces;
- golden-output tests for example programs;
- malformed bytecode regression tests;
- verifier and trace documentation.

## v6

Implemented:

- optional Rust terminal UI under `tui/`;
- terminal-native program browser;
- TUI actions for `run`, `trace`, `verify`, `dis`, and `info`;
- debugger handoff from the TUI through `x`;
- pure ANSI/stty terminal handling with no React, browser wrapper, or external Rust crates;
- optional CMake target `waivm_tui`;
- CI cargo check for the TUI.

## Later Candidates

Do not treat these as implemented:

- inline source editing in the terminal UI;
- source-level debug symbols;
- call frames and a real VM calling convention;
- static data section in `.waibc`;
- richer memory operand syntax;
- function arguments/returns beyond raw registers and stack;
- Windows x64 runtime;
- JIT compilation;
- heap allocation or garbage collection.
