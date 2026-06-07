# Architecture

waivm v0.1 is split into a C tooling layer and a NASM execution layer.

## Components

| Component | Responsibility |
|---|---|
| `src/cli.c` | command parsing and user-facing errors |
| `src/assembler.c` | `.wai` parsing, label collection, label resolution |
| `src/bytecode.c` | in-memory program storage |
| `src/vm.c` | VM initialization and C/ASM boundary |
| `asm/vm_linux_x86_64.asm` | bytecode dispatch and instruction execution |

## Execution model

1. CLI reads a `.wai` source file.
2. The C assembler emits a `wai_program` containing fixed-width instructions.
3. `wai_vm_init` binds the instruction array to VM state.
4. `wai_vm_exec_asm` runs the dispatch loop in NASM.
5. `print` calls back into `wai_vm_emit_print` so output remains testable.

## Scope boundary

Persistent `.waibc` files, the disassembler, and the debugger are later milestones. v0.1 keeps the runtime small enough to audit.
