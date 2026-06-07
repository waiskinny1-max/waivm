# Bytecode Verifier

`waivm verify` validates a source or bytecode program without executing it.

```sh
waivm verify examples/sum.wai
waivm asm examples/sum.wai -o sum.waibc
waivm verify sum.waibc
```

The verifier checks the structural safety of the instruction stream:

- known opcode range;
- reserved instruction fields are zero;
- register operands are within `r0` through `r7`;
- jump and call targets are inside the instruction stream;
- absolute memory operands fit inside the 64 KiB VM memory;
- immediate shift counts are within `0..63`.

The verifier is intentionally conservative and format-level. It does not prove that a program terminates, that every dynamic stack operation is balanced, or that a register-derived memory address will be safe at runtime. Those checks remain runtime checks.

Example success output:

```text
verify: ok - ok: 8 instructions verified
```

Example failure output:

```text
verify error at ip=3: bad jump target (jump target 99 is outside 0..7)
```
