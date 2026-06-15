# Contributing to waivm

`waivm` is a small VM project, but it crosses several sharp boundaries: source parsing, bytecode encoding, verifier rules, a handwritten assembly runtime, a C debugger, and a Rust terminal UI. Treat changes as systems work, not demo code.

## Local setup

Required for the core project:

- CMake 3.20+
- C17 compiler
- NASM
- Make or Ninja
- Python 3

Optional for terminal UI work:

- Rust toolchain with `cargo`, `rustfmt`, and `clippy`

## Before opening a pull request

Run:

```bash
python3 scripts/quality_gate.py --full
```

For native sanitizer checks:

```bash
python3 scripts/quality_gate.py --sanitizers --skip-tui
```

For a quick pre-commit pass:

```bash
python3 scripts/quality_gate.py --fast
```

## Review checklist

A change should answer these questions clearly:

- What behavior changed?
- Which input is trusted and which is untrusted?
- What happens on malformed `.wai` or `.waibc` input?
- Are errors specific enough to debug without rerunning under a debugger?
- Did the change affect bytecode format, verifier rules, instruction semantics, or VM ABI?
- Are tests added for the failure path, not only the happy path?
- If the change touches a hot path, is there a benchmark or a reason benchmarking is irrelevant?
- If docs mention a feature, does the code implement it today?

## Commit discipline

Prefer small commits grouped by behavior:

1. implementation,
2. tests,
3. docs,
4. benchmark or tooling changes.

Do not mix a formatting-only sweep with semantic changes. It makes review worse and hides bugs.

## Documentation discipline

Roadmap items belong in `docs/roadmap.md`. Current-feature claims belong in `README.md` only when the command or behavior is implemented and tested.
