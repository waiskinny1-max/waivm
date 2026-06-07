# Instruction Set

waivm uses 8 signed 64-bit registers: `r0` through `r7`.

## Instructions

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
| `jmp label` | `JMP` | unconditional jump |
| `jz rX, label` | `JZ` | jump if `rX == 0` |
| `jnz rX, label` | `JNZ` | jump if `rX != 0` |
| `print rX` | `PRINT` | print register value |
| `halt` | `HALT` | stop execution |

## Flags

`zf` is updated by move and arithmetic instructions. Branch instructions in v2 use explicit register checks rather than `zf`, because the source syntax makes the condition visible:

```asm
jnz r0, loop
```

## Errors

The VM reports structured errors for:

- unknown opcode;
- instruction pointer out of bounds;
- invalid register index;
- division by zero;
- invalid jump target.
