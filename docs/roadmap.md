# Roadmap

## v0.1

Implemented:

- NASM execution loop;
- C17 `.wai` assembler;
- register arithmetic and branching;
- example programs;
- smoke tests.

## v2

Implemented:

- `.waibc` bytecode writer;
- `.waibc` bytecode loader;
- bytecode metadata `info` command;
- disassembler;
- interactive debugger;
- debugger stepping and breakpoints;
- additional tests.

## v0.3 Candidates

- better diagnostics with source spans;
- labels preserved in disassembly sidecar data;
- optional memory instructions;
- richer CLI integration tests;
- release artifacts in CI.

## Explicitly Out of Scope Before v1.0

- JIT compiler;
- garbage collector;
- object system;
- strings;
- networking;
- package manager;
- self-hosted compiler;
- assembler written in assembly;
- web UI.
