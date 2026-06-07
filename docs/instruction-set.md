# Instruction Set

waivm v4 uses 8 signed 64-bit registers: `r0` through `r7`.

All instructions are encoded as fixed-width 12-byte records. Labels exist only at the `.wai` source level; the assembler resolves labels into absolute instruction indexes.

## Arithmetic

| Source | Bytecode form | Meaning |
|---|---|---|
| `mov rX, IMM` | `MOV_IMM` | store immediate in register |
| `mov rX, rY` | `MOV_REG` | copy register |
| `add rX, IMM/rY` | `ADD_*` | add into `rX` |
| `sub rX, IMM/rY` | `SUB_*` | subtract from `rX` |
| `mul rX, IMM/rY` | `MUL_*` | multiply `rX` |
| `div rX, IMM/rY` | `DIV_*` | signed integer division |
| `mod rX, IMM/rY` | `MOD_*` | signed integer remainder |

Division and modulo by zero fail with `WAI_ERR_DIV_ZERO`.

## Bitwise

| Source | Bytecode form | Meaning |
|---|---|---|
| `and rX, IMM/rY` | `AND_*` | bitwise AND |
| `or rX, IMM/rY` | `OR_*` | bitwise OR |
| `xor rX, IMM/rY` | `XOR_*` | bitwise XOR |
| `not rX` | `NOT` | bitwise NOT |
| `shl rX, IMM/rY` | `SHL_*` | logical left shift |
| `shr rX, IMM/rY` | `SHR_*` | logical right shift |

Shift counts must be in the range `0..63`. Invalid shift counts fail with `WAI_ERR_BAD_SHIFT`.

## Memory

The VM has 64 KiB of byte-addressed memory. `load` and `store` operate on signed 64-bit little-endian values.

| Source | Bytecode form | Meaning |
|---|---|---|
| `load rX, [ADDR]` | `LOAD_ABS` | load from absolute memory address |
| `load rX, [rY]` | `LOAD_REG` | load from address held in `rY` |
| `store [ADDR], rX` | `STORE_ABS` | store to absolute memory address |
| `store [rY], rX` | `STORE_REG` | store to address held in `rY` |

Valid 64-bit memory access addresses are `0..65528`.

## Stack and Calls

The VM stack lives inside the same 64 KiB memory region and grows downward from address `65536`.

| Source | Bytecode form | Meaning |
|---|---|---|
| `push rX` | `PUSH` | decrement `sp` by 8 and store `rX` |
| `pop rX` | `POP` | load value at `sp`, then increment `sp` by 8 |
| `call label` | `CALL` | push return address and jump to label |
| `ret` | `RET` | pop return address and jump to it |

There are no stack frames, local variables, or argument conventions yet.

## Branch and Compare

| Source | Bytecode form | Meaning |
|---|---|---|
| `cmp rX, IMM` | `CMP_IMM` | set zero flag if equal |
| `cmp rX, rY` | `CMP_REG` | set zero flag if equal |
| `jmp label` | `JMP` | unconditional jump |
| `jz rX, label` | `JZ` | jump if `rX == 0` |
| `jnz rX, label` | `JNZ` | jump if `rX != 0` |
| `je label` | `JE` | jump if zero flag is set |
| `jne label` | `JNE` | jump if zero flag is clear |

`jz` and `jnz` are register-based branches retained for simple loops. `je` and `jne` are flag-based branches intended to pair with `cmp`.

## I/O and Halt

| Source | Bytecode form | Meaning |
|---|---|---|
| `nop` | `NOP` | no operation |
| `print rX` | `PRINT` | emit signed decimal integer |
| `halt` | `HALT` | stop execution |
