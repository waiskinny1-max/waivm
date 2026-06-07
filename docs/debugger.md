# Debugger

The debugger is an interactive REPL around the C stepping engine.

```sh
./build/waivm debug examples/call.wai
```

## Commands

| Command | Meaning |
|---|---|
| `help` | show available commands |
| `regs` | print registers, `ip`, `sp`, `zf`, halt state, and print count |
| `ip` | print current instruction pointer |
| `dis` | disassemble around current `ip` |
| `mem <addr> [bytes]` | dump up to 256 bytes of VM memory |
| `stack [count]` | dump stack qwords from `sp` |
| `step` / `s` | execute one instruction |
| `continue` / `c` | run until halt, error, or breakpoint |
| `break <ip>` / `b <ip>` | set breakpoint at instruction index |
| `clear <ip>` | remove breakpoint |
| `quit` / `q` | exit debugger |

## Example Session

```text
waidbg> dis
> 0000  mov r0, 21
  0001  call 4
  0002  print r0
waidbg> break 2
breakpoint set at 2
waidbg> c
breakpoint hit at 2
waidbg> regs
r0=42  r1=0  r2=0  r3=0  r4=0  r5=0  r6=0  r7=0
ip=2 sp=65536 zf=0 halted=0 prints=0
```

The debugger intentionally uses C stepping rather than the NASM run loop. This keeps breakpoint handling and inspection simple while preserving the assembly runtime for normal execution.
