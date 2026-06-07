# Instruction Set

waivm v3 uses 8 signed 64-bit registers: `r0` through `r7`.

## Core Register Instructions

| Source form | Internal opcode | Effect |
|---|---|---|
| `mov rX, IMM` | `MOV_IMM` | `rX = IMM` |
| `mov rX, rY` | `MOV_REG` | `rX = rY` |
| `add rX, IMM` | `ADD_IMM` | `rX += IMM` |
| `add rX, rY` | `ADD_REG` | `rX += rY` |
| `sub rX, IMM` | `SUB_IMM` | `rX -= IMM` |
| `sub rX, rY` | `SUB_REG` | `rX -= rY` |
| `mul rX, IMM` | `MUL_IMM` | `rX *= IMM` |
| `mul rX, rY` | `MUL_REG` | `rX *= rY` |
| `div rX, IMM` | `DIV_IMM` | `rX /= IMM` |
| `div rX, rY` | `DIV_REG` | `rX /= rY` |

Division is signed integer division. Division by zero is a runtime error.

## Memory Instructions

The VM has 64 KiB of byte-addressed memory. `load` and `store` operate on signed 64-bit little-endian values.

| Source form | Internal opcode | Effect |
|---|---|---|
| `load rX, [ADDR]` | `LOAD_ABS` | `rX = *(i64 *)(memory + ADDR)` |
| `load rX, [rY]` | `LOAD_REG` | `rX = *(i64 *)(memory + rY)` |
| `store [ADDR], rX` | `STORE_ABS` | `*(i64 *)(memory + ADDR) = rX` |
| `store [rY], rX` | `STORE_REG` | `*(i64 *)(memory + rY) = rX` |

Valid load/store addresses are `0` through `65528`, inclusive. Accesses outside that range fail with `WAI_ERR_MEMORY_OOB`.

## Stack and Calls

The VM stack lives inside the same 64 KiB memory region and grows downward from address `65536`.

| Source form | Internal opcode | Effect |
|---|---|---|
| `push rX` | `PUSH` | decrement `sp` by 8 and store `rX` |
| `pop rX` | `POP` | load value at `sp`, increment `sp` by 8, store into `rX` |
| `call label` | `CALL` | push return address and jump to label |
| `ret` | `RET` | pop return address and jump to it |

This is a raw VM stack, not a high-level call-frame ABI. There are no named local variables, frame pointers, or calling convention rules beyond the stack behavior above.

## Branch and Compare

| Source form | Internal opcode | Effect |
|---|---|---|
| `cmp rX, IMM` | `CMP_IMM` | `zf = (rX == IMM)` |
| `cmp rX, rY` | `CMP_REG` | `zf = (rX == rY)` |
| `jmp label` | `JMP` | unconditional jump |
| `jz rX, label` | `JZ` | jump if `rX == 0` |
| `jnz rX, label` | `JNZ` | jump if `rX != 0` |
| `je label` | `JE` | jump if `zf == 1` |
| `jne label` | `JNE` | jump if `zf == 0` |

`jz` and `jnz` are register-based branches retained for simple loops. `je` and `jne` are flag-based branches intended to pair with `cmp`.

## I/O and Halt

| Source form | Internal opcode | Effect |
|---|---|---|
| `print rX` | `PRINT` | print register value as decimal integer |
| `halt` | `HALT` | stop execution |

## Errors

The VM reports structured errors for:

- unknown opcode;
- instruction pointer out of bounds;
- invalid register index;
- division by zero;
- invalid jump target;
- memory access out of bounds;
- stack overflow;
- stack underflow.
