# Architecture

waivm is a small register-based bytecode VM designed to show the boundary between low-level execution and systems tooling.

## Components

| Component | Language | Role |
|---|---|---|
| NASM runtime | x86-64 assembly | normal VM dispatch loop |
| VM state | C ABI struct | registers, IP, flags, code pointer, output state |
| assembler | C17 | parse `.wai` source, resolve labels, emit instructions |
| bytecode loader/writer | C17 | read and write `.waibc` files |
| disassembler | C17 | print readable instructions from encoded programs |
| debugger | C17 | command loop, stepping, register dump, breakpoints |
| CLI | C17 | user-facing commands |

## VM State

The VM has 8 signed 64-bit registers and an instruction pointer. The instruction stream is an array of fixed-width 12-byte instructions.

```text
r0-r7   signed 64-bit general-purpose registers
ip      absolute instruction index
zf      zero flag maintained by arithmetic/move operations
halted  halt state
```

## Runtime Boundary

Normal execution flows through `wai_vm_execute`, which calls `wai_vm_exec_asm` in `asm/vm_linux_x86_64.asm`.

The C side prepares the program and initializes `wai_vm`. The assembly side reads the VM struct fields directly using ABI offsets checked by `_Static_assert` in `src/vm.c`.

The only C callback used by the assembly runtime is `wai_vm_emit_print`, which centralizes output and lets tests suppress stdout while checking the last printed value.

## Debugger Boundary

The debugger uses `wai_vm_step` in C instead of the assembly loop. This keeps debugger behavior inspectable and avoids mixing interactive control flow into the assembly dispatch loop.
