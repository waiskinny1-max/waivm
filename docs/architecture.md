# Architecture

waivm is split into a small assembly execution core and C17 tooling.

## Components

| Component | Role |
|---|---|
| NASM runtime | normal bytecode dispatch loop and instruction execution |
| C VM state | ABI-shared state object consumed by assembly and C stepping |
| Assembler | parse `.wai`, resolve labels, emit instruction records |
| Bytecode loader/writer | read and write `.waibc` files |
| Disassembler | render bytecode back into readable assembly-like text |
| Debugger | interactive stepping, breakpoints, registers, memory, stack |
| CLI | command routing and file type detection |

The normal `run` path uses `wai_vm_exec_asm`. The debugger uses `wai_vm_step`, because debugger control requires stopping between instructions and inspecting state.

## VM State

The VM state contains:

- 8 signed 64-bit registers;
- instruction pointer `ip`;
- zero flag `zf`;
- halt flag;
- pointer to instruction array;
- code count;
- error code;
- print bookkeeping;
- 64 KiB memory;
- stack pointer `sp`.

The C layout is part of the assembly ABI and is documented in `docs/vm-abi.md`.

## Execution Model

Instructions are fixed-width 12-byte records. The NASM loop fetches `code[ip]`, increments `ip`, dispatches on `opcode`, and mutates the VM state. Branches, calls, and returns overwrite `ip` when needed.

## Memory and Stack

Memory is byte-addressed but v4 exposes only 64-bit `load` and `store` instructions. The valid address range for those operations is `0..65528`.

The stack shares the same 64 KiB memory region and grows downward from address `65536`. It stores raw 64-bit values. There are no stack frames or local-variable conventions yet.

## Error Model

The runtime records errors inside the VM state and returns a nonzero status from the assembly entrypoint. C converts those codes into user-facing messages.

Important error cases:

- invalid opcode;
- instruction pointer out of bounds;
- bad register operand;
- division/modulo by zero;
- bad jump target;
- memory access out of bounds;
- stack overflow/underflow;
- invalid shift count.
