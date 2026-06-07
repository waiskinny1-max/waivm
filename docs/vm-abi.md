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
