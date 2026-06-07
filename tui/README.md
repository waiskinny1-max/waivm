# waivm-tui

`waivm-tui` is a terminal-native control surface for `waivm`.

It is intentionally not a React terminal UI, not a browser wrapper, and not a web dashboard. It is a small Rust terminal application that drives the existing C/NASM `waivm` binary.

## Features

- browse example `.wai` and `.waibc` programs
- run programs from the terminal UI
- trace programs interactively through the same interface
- verify bytecode/source files
- disassemble selected programs
- show bytecode metadata through the `info` command
- leave the TUI temporarily and open the existing interactive debugger
- pure ANSI/stty terminal handling with no runtime dependencies

## Build

Build the VM first:

```sh
cmake -S . -B build
cmake --build build
```

Build the terminal UI:

```sh
cargo build --manifest-path tui/Cargo.toml --release
```

Run it from the repository root:

```sh
./tui/target/release/waivm-tui --root . --waivm ./build/waivm
```

You can also set the VM binary through the environment:

```sh
WAIVM_BIN=./build/waivm ./tui/target/release/waivm-tui
```

## Optional CMake target

The Rust TUI is not built by default. To request it through CMake:

```sh
cmake -S . -B build -DWAI_BUILD_TUI=ON
cmake --build build --target waivm_tui
```

## Keyboard

```text
Up/Down or k/j     select program
Left/Right or [/]  change action
Enter              execute selected action
r/t/v/d/i          run/trace/verify/disassemble/info
x                  temporarily leave TUI and open waivm debug
PageUp/PageDown    scroll output
c                  clear output
?                  toggle help
q or Esc           quit
```

## Architecture

The TUI is deliberately a separate tool. It does not link directly against the VM internals. Instead, it invokes the public `waivm` CLI. That keeps the core VM stable, makes the interface dogfood the actual command-line API, and avoids duplicating execution logic in Rust.
