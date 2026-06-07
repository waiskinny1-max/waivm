# Trace Mode

`waivm trace` executes a source or bytecode program one instruction at a time and prints the VM state after each step.

```sh
waivm trace examples/factorial.wai
```

Trace mode is meant for demonstrations, regression debugging, and understanding the execution model. Normal `waivm run` still uses the NASM execution core. Trace mode uses the C stepping engine, like the debugger, because it needs instruction-by-instruction visibility.

Each trace row includes:

- instruction pointer before execution;
- opcode name;
- encoded operands;
- next instruction pointer;
- zero flag;
- stack pointer;
- all eight registers;
- printed value, if the instruction produced output.

Example excerpt:

```text
trace start: 8 instructions
0000  mov.imm  a=0 b=0 imm=5
      => ip=1 zf=0 sp=65536 r0=5 r1=0 r2=0 r3=0 r4=0 r5=0 r6=0 r7=0
...
0006  print    a=1 b=0 imm=0
      => ip=7 zf=1 sp=65536 r0=0 r1=120 r2=0 r3=0 r4=0 r5=0 r6=0 r7=0 print=120
trace end: 25 steps, 1 prints
```

A hard step limit prevents accidental infinite traces from producing unbounded output.
