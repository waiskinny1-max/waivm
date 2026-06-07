# Architecture

waivm is split into a small assembly execution core and C17 tooling.

## Runtime Split

| Layer | Responsibility |
|---|---|
| NASM runtime | normal bytecode dispatch loop and instruction execution |
| C VM stepper | single-instruction stepping for debugger UX |
| C assembler | `.wai` parsing, label resolution, instruction encoding |
| C bytecode module | `.waibc` read/write/validation |
| C disassembler | readable instruction listing |
| C debugger | REPL, stepping, register dump, memory dump, stack dump, breakpoints |
| CLI | command routing and file type detection |

The normal `run` path uses `wai_vm_exec_asm`. The debugger uses `wai_vm_step`, because debugger control requires stopping between instructions and inspecting state.

## VM State

The VM state contains:

- `r0` through `r7`, signed 64-bit registers;
- `ip`, absolute instruction index;
- `zf`, zero flag;
- `halted`, stop marker;
- pointer to fixed-width encoded instructions;
- 64 KiB linear memory;
- `sp`, stack pointer growing downward from `65536`;
- error code and print bookkeeping.

## Execution Model

Each instruction is a packed 12-byte record:

```c
typedef struct __attribute__((packed)) wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    int64_t imm;
} wai_instruction;
```

The NASM loop fetches `code[ip]`, increments `ip`, dispatches on `opcode`, and mutates the VM state. Branches, calls, and returns overwrite `ip` when needed.

## Memory and Stack

Memory is byte-addressed but v3 exposes only 64-bit `load` and `store` instructions. The valid address range for those operations is `0..65528`.

The stack shares the same 64 KiB memory region and grows downward from address `65536`. It stores raw 64-bit values. There are no stack frames or local-variable conventions yet.

## Toolchain Flow

```text
.wai source
  -> assembler
  -> wai_program in memory
  -> optional .waibc writer
  -> NASM VM runtime or C debugger stepper
```

`.waibc` exists to make the project feel like a real toolchain, not just a source interpreter.
