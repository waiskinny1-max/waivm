# VM ABI

The NASM runtime and C code share the `wai_vm` and `wai_instruction` layouts.

Target ABI: System V x86-64 on Linux.

Entry point:

```c
int wai_vm_exec_asm(wai_vm *vm);
```

Return value:

- `0`: success;
- nonzero: runtime error, with `vm->error` set.

## VM Layout

The assembly runtime assumes the field order defined in `include/wai/vm.h`:

| Field | Purpose |
|---|---|
| `regs[8]` | signed 64-bit general-purpose VM registers |
| `ip` | instruction pointer measured in instruction indexes |
| `zf` | zero flag |
| `halted` | halt state |
| `code` | pointer to packed instruction array |
| `code_count` | instruction count |
| `error` | runtime error code |
| `print_stream` | C-managed stream for print hook |
| `last_print` | last printed value, used by tests |
| `print_count` | number of emitted print values |
| `memory[65536]` | byte-addressed VM memory |
| `sp` | stack pointer inside VM memory |

## Instruction Layout

The instruction struct is packed and asserted to be exactly 12 bytes.

| Field | Size |
|---|---:|
| `opcode` | 1 byte |
| `a` | 1 byte |
| `b` | 1 byte |
| `c` | 1 byte |
| `imm` | 8 bytes |

## Stack Convention

`sp` starts at `65536`. Stack operations store 64-bit values in VM memory and move `sp` by 8 bytes:

- `push`: `sp -= 8`, then store value at `memory[sp]`;
- `pop`: load value at `memory[sp]`, then `sp += 8`.

`call` pushes the return instruction index and jumps to an absolute instruction index. `ret` pops the return instruction index and jumps to it.

There is no frame pointer or local-variable convention yet.

## Shift Semantics

`shl` and `shr` accept shift counts from `0` through `63`. `shr` is logical and treats the register value as an unsigned 64-bit bit pattern before storing the result back as a signed 64-bit VM value.
