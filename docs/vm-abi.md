# VM ABI

The NASM runtime depends on stable C struct offsets.

## `wai_instruction`

| Offset | Field | Size |
|---:|---|---:|
| 0 | `opcode` | 1 |
| 1 | `a` | 1 |
| 2 | `b` | 1 |
| 3 | `c` | 1 |
| 4 | `imm` | 8 |

Total size: 12 bytes.

## `wai_vm`

| Offset | Field |
|---:|---|
| 0 | `regs[8]` |
| 64 | `ip` |
| 72 | `zf` |
| 73 | `halted` |
| 80 | `code` |
| 88 | `code_count` |
| 96 | `error` |

`src/vm.c` contains `_Static_assert` checks for these offsets.

## Call boundary

`wai_vm_exec_asm(wai_vm *vm)` follows the Linux x86-64 System V ABI.

The assembly runtime may call:

```c
void wai_vm_emit_print(wai_vm *vm, wai_value value);
```

The assembly core aligns the stack before this external call.
