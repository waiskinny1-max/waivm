# Roadmap

## v0.1 — Assembly execution core

Implemented target:

- Linux x86-64 NASM execution loop
- C17 `.wai` source assembler
- register VM with `r0` through `r7`
- `MOV`, `ADD`, `SUB`, `MUL`, `DIV`, `JMP`, `JZ`, `JNZ`, `PRINT`, `HALT`
- examples for sum, factorial, Fibonacci, loop, and branch
- minimal CTest suite

## v0.2 — Cleaner runtime API

- single-step execution entry point
- better error locations
- optional trace mode

## v0.3 — Bytecode files + disassembler

- write `.waibc`
- read `.waibc`
- validate `WAI0` header
- `waivm asm`
- `waivm dis`
- `waivm info`

## v0.4 — Debugger

- breakpoints
- register dump
- disassemble around IP
- continue and step

## v0.5 — CI and polish

- GitHub Actions
- broader tests
- README screenshots
- release artifact

## v1.0 — Portfolio release

- stable bytecode format
- stable instruction set
- complete docs
- tagged release
