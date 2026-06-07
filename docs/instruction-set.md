# Instruction Set

waivm v0.1 uses signed 64-bit integer registers and fixed-width internal bytecode.

## Registers

- `r0` through `r7`
- each register is a signed 64-bit integer
- invalid registers are rejected by the assembler or runtime

## Instructions

| Source syntax | Runtime behavior |
|---|---|
| `mov rX, IMM` | `rX = IMM` |
| `mov rX, rY` | `rX = rY` |
| `add rX, IMM` | `rX = rX + IMM` |
| `add rX, rY` | `rX = rX + rY` |
| `sub rX, IMM` | `rX = rX - IMM` |
| `sub rX, rY` | `rX = rX - rY` |
| `mul rX, IMM` | `rX = rX * IMM` |
| `mul rX, rY` | `rX = rX * rY` |
| `div rX, IMM` | `rX = rX / IMM` |
| `div rX, rY` | `rX = rX / rY` |
| `jmp label` | set `ip` to label target |
| `jz rX, label` | jump when `rX == 0` |
| `jnz rX, label` | jump when `rX != 0` |
| `print rX` | print `rX` as decimal integer |
| `halt` | terminate execution successfully |

Division is signed integer division. Division by zero is a runtime error.
