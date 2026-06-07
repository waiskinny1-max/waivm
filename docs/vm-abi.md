# VM ABI

`asm/vm_linux_x86_64.asm` reads the C `wai_vm` layout directly. The ABI offsets are guarded by `_Static_assert` in `src/vm.c`.

## Function Boundary

```c
int wai_vm_exec_asm(wai_vm *vm);
```

System V AMD64 ABI:

- `rdi` receives `wai_vm *`;
- return value is placed in `eax`;
- callee preserves non-volatile registers.

## `wai_vm` Offsets

| Field | Offset |
|---|---:|
| `regs` | 0 |
| `ip` | 64 |
| `zf` | 72 |
| `halted` | 73 |
| `code` | 80 |
| `code_count` | 88 |
| `error` | 96 |
| `memory` | 128 |
| `sp` | 65664 |

The assembly runtime does not access `print_stream`, `last_print`, or `print_count` directly. It calls `wai_vm_emit_print(vm, value)`.

## Instruction Layout

Each instruction is 12 bytes:

| Field | Offset |
|---|---:|
| `opcode` | 0 |
| `a` | 1 |
| `b` | 2 |
| `c` | 3 |
| `imm` | 4 |

The instruction struct is packed in C and asserted to be exactly 12 bytes.

## Stack Layout

`sp` starts at `65536`. Stack operations store 64-bit values in VM memory and move `sp` by 8 bytes:

- `push`: `sp -= 8`, then store value at `memory[sp]`;
- `pop`: load value at `memory[sp]`, then `sp += 8`.

`call` pushes the return instruction index and jumps to an absolute instruction index. `ret` pops the return instruction index and jumps to it.
