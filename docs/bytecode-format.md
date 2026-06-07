# Bytecode Format

waivm bytecode files use the `.waibc` extension.

## Endianness

All multi-byte numeric fields are little-endian.

## File Layout

```text
+----------------------+ 0
| 32-byte header       |
+----------------------+ 32
| instruction 0        | 12 bytes
+----------------------+
| instruction 1        | 12 bytes
+----------------------+
| ...                  |
+----------------------+
```

## Header

| Offset | Size | Field | Value in v3 |
|---:|---:|---|---|
| 0 | 4 | magic | `WAI0` |
| 4 | 2 | version | `3` |
| 6 | 2 | header size | `32` |
| 8 | 2 | instruction size | `12` |
| 10 | 2 | register count | `8` |
| 12 | 4 | flags | `0` |
| 16 | 8 | code count | number of instructions |
| 24 | 8 | data size | `0` |

`data_size` remains zero in v3. The 64 KiB VM memory is runtime state, not serialized static data.

## Instruction Encoding

Each instruction is exactly 12 bytes:

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 1 | opcode | instruction selector |
| 1 | 1 | `a` | destination/source/condition register |
| 2 | 1 | `b` | source/address register |
| 3 | 1 | `c` | reserved, currently zero |
| 4 | 8 | `imm` | signed immediate, absolute memory address, or absolute instruction index |

## Opcode Map

| Opcode | Name |
|---:|---|
| 1 | `MOV_IMM` |
| 2 | `MOV_REG` |
| 3 | `ADD_IMM` |
| 4 | `ADD_REG` |
| 5 | `SUB_IMM` |
| 6 | `SUB_REG` |
| 7 | `MUL_IMM` |
| 8 | `MUL_REG` |
| 9 | `DIV_IMM` |
| 10 | `DIV_REG` |
| 11 | `JMP` |
| 12 | `JZ` |
| 13 | `JNZ` |
| 14 | `PRINT` |
| 15 | `HALT` |
| 16 | `LOAD_ABS` |
| 17 | `LOAD_REG` |
| 18 | `STORE_ABS` |
| 19 | `STORE_REG` |
| 20 | `PUSH` |
| 21 | `POP` |
| 22 | `CALL` |
| 23 | `RET` |
| 24 | `CMP_IMM` |
| 25 | `CMP_REG` |
| 26 | `JE` |
| 27 | `JNE` |

## Validation

The loader rejects:

- bad magic bytes;
- unsupported version;
- wrong header size;
- wrong instruction size;
- wrong register count;
- non-zero data size;
- trailing bytes after the encoded program;
- invalid opcodes;
- invalid register operands;
- jump or call targets outside the program.

Memory bounds are checked at runtime because register-addressed memory operands cannot be fully validated at load time.
