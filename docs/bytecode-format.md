# Bytecode Format

`.waibc` is waivm's persistent bytecode format.

v4 deliberately uses fixed-width records. This keeps the NASM dispatch loop simple and makes corruption handling easier.

## Endianness

All multi-byte integers are little-endian.

## Header

Header size: 32 bytes.

| Offset | Size | Field | Value in v4 |
|---:|---:|---|---|
| 0 | 4 | magic | `WAI0` |
| 4 | 2 | version | `5` |
| 6 | 2 | header size | `32` |
| 8 | 2 | instruction size | `12` |
| 10 | 2 | register count | `8` |
| 12 | 4 | flags | `0` |
| 16 | 8 | code count | number of instructions |
| 24 | 8 | data size | `0` |

`data_size` remains zero in v4. The 64 KiB VM memory is runtime state, not serialized static data.

## Instruction Record

Instruction size: 12 bytes.

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 1 | opcode | numeric opcode |
| 1 | 1 | a | first register operand |
| 2 | 1 | b | second register operand |
| 3 | 1 | c | reserved, currently zero |
| 4 | 8 | imm | signed immediate, memory address, or jump target |

The C representation is packed:

```c
typedef struct wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    int64_t imm;
} wai_instruction;
```

## Opcode Versioning

v4 extends v3 with:

- `NOP`
- `MOD_IMM`, `MOD_REG`
- `AND_IMM`, `AND_REG`
- `OR_IMM`, `OR_REG`
- `XOR_IMM`, `XOR_REG`
- `NOT`
- `SHL_IMM`, `SHL_REG`
- `SHR_IMM`, `SHR_REG`

Because the bytecode version is exact-match validated, older `.waibc` files must be reassembled when the bytecode version changes.

## Validation

The loader rejects:

- bad magic;
- unsupported bytecode version;
- wrong header size;
- wrong instruction size;
- wrong register count;
- nonzero data size;
- trailing bytes;
- unknown opcodes;
- invalid register operands;
- jump targets outside the code range;
- absolute memory addresses outside the valid 64-bit access range;
- immediate shift counts outside `0..63`.

Register-addressed memory operands and register-based shift counts are checked at runtime.
