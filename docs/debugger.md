# Debugger

The debugger is intentionally small but usable. It runs bytecode through the C stepping engine rather than the NASM loop so it can stop between instructions.

Start it:

```sh
waivm debug examples/sum.wai
```

## Commands

| Command | Meaning |
|---|---|
| `help` | show available commands |
| `regs` | print registers, `ip`, `sp`, `zf`, halt state, and print count |
| `ip` | print the current instruction pointer |
| `dis` | disassemble a small window around `ip` |
| `mem <addr> [bytes]` | dump up to 256 memory bytes |
| `stack [count]` | dump stack qwords from `sp` |
| `step` / `s` | execute one instruction |
| `continue` / `c` | run until halt, error, or breakpoint |
| `break <ip>` / `b <ip>` | set breakpoint |
| `clear <ip>` | clear breakpoint |
| `quit` / `q` | exit debugger |

## Example Session

```text
waivm debugger. type 'help' for commands.
waidbg> dis
> 0000  mov r0, 10
  0001  mov r1, 0
  0002  add r1, r0
waidbg> break 5
breakpoint set at 5
waidbg> continue
breakpoint hit at 5
waidbg> regs
r0=0  r1=55  r2=0  r3=0  r4=0  r5=0  r6=0  r7=0
ip=5 sp=65536 zf=1 halted=0 prints=0
```

## Notes

The debugger has no source-level symbol table yet. Breakpoints use instruction indexes shown by `dis`.
