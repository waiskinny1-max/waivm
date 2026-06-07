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

| Offset | Size | Field | Value in v1 |
|---:|---:|---|---|
| 0 | 4 | magic | `WAI0` |
| 4 | 2 | version | `1` |
| 6 | 2 | header size | `32` |
| 8 | 2 | instruction size | `12` |
| 10 | 2 | register count | `8` |
| 12 | 4 | flags | `0` |
| 16 | 8 | code count | number of instructions |
| 24 | 8 | data size | `0` |

## Instruction Encoding

Each instruction is exactly 12 bytes:

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 1 | opcode | instruction selector |
| 1 | 1 | `a` | destination or condition register |
| 2 | 1 | `b` | source register |
| 3 | 1 | `c` | reserved, currently zero |
| 4 | 8 | `imm` | signed immediate or absolute instruction index |

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
- jump targets outside the program.

The format is intentionally plain. It is built for inspectability and for a simple assembly dispatch loop, not compression.
