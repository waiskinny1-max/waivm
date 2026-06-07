# Bytecode Format

v0.1 does not write `.waibc` files yet.

The internal instruction encoding is already fixed so the NASM runtime can stay simple:

```c
typedef struct wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    int64_t imm;
} wai_instruction;
```

The packed size is 12 bytes.

## Planned `.waibc` header for v0.3

The planned persistent bytecode format starts with:

| Field | Size | Purpose |
|---|---:|---|
| magic | 4 | `WAI0` |
| version | 2 | bytecode version |
| register_count | 2 | expected register count |
| flags | 4 | format flags |
| code_size | 8 | instruction byte length |
| data_size | 8 | reserved data section length |

This document intentionally marks file persistence as future work.
