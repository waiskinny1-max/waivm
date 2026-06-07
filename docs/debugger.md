# Debugger

The debugger is interactive and source-format agnostic. It can load either `.wai` source or `.waibc` bytecode.

```sh
waivm debug examples/sum.wai
waivm debug sum.waibc
```

## Commands

| Command | Alias | Meaning |
|---|---|---|
| `help` | | show debugger commands |
| `regs` | | print all registers and VM state |
| `ip` | | print the current instruction pointer |
| `dis` | | disassemble around the current instruction pointer |
| `step` | `s` | execute one instruction |
| `continue` | `c` | run until halt, error, or breakpoint |
| `break <ip>` | `b <ip>` | set breakpoint at instruction index |
| `clear <ip>` | | clear breakpoint |
| `quit` | `q` | exit debugger |

## Execution Model

Normal `waivm run` execution uses the NASM dispatch loop.

The debugger uses the C stepping engine. This is deliberate: stepping, breakpoints, and register inspection require a clean stop between instructions. The C stepping engine mirrors the NASM instruction semantics and shares the same `wai_vm` state structure.

## Example

```text
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
ip=5 zf=1 halted=0 prints=0
```
