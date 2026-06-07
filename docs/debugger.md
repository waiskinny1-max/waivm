# Debugger

The debugger is not implemented in v0.1.

Planned commands:

| Command | Purpose |
|---|---|
| `step` / `s` | execute one instruction |
| `continue` / `c` | run until halt or breakpoint |
| `regs` | print registers |
| `break <ip>` | set instruction breakpoint |
| `delete <ip>` | remove breakpoint |
| `dis` | show instructions around current IP |
| `mem` | placeholder for future memory dump |
| `quit` / `q` | exit debugger |

The debugger will sit in C and drive a step-capable execution interface. The v0.1 assembly function executes continuously until halt or error.
