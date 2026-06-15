# CODESKILL review: waivm

Review date: 2026-06-15
Repo reviewed: `waiskinny1-max/waivm`

## Verdict

`waivm` is strong for a young systems repo: it has a real C/ASM/Rust shape, documented bytecode behavior, tests, examples, a verifier, trace mode, and a terminal UI. It does not look like a hollow README-only project.

It still does **not** fully satisfy the strict CODESKILL standard until the repo owns its quality policy and merge gates. The missing piece is not another feature. The missing piece is enforcement.

## What already fits

- C17 core with a handwritten NASM execution loop.
- CMake build with strict warning flags.
- Tests directory exists for assembler, bytecode, debugger, disassembler, and VM behavior.
- README documents current implemented limits and not-implemented items.
- Optional Rust TUI is separated from the C/ASM core.

## Gaps fixed by this patch

- Adds a repo-specific code-quality policy.
- Adds a local quality gate script that checks Python, shell, whitespace, native build/tests, and optional Rust TUI checks.
- Adds GitHub Actions CI for Debug, Release, sanitizer, Rust TUI, and script hygiene.
- Adds benchmark harness for representative CLI latency and regression comparison.
- Adds formatting/editor configs and pre-commit hooks.
- Adds contribution rules that force reviewers to check malformed input, verifier/runtime behavior, docs truth, and performance evidence.

## Still not proven by this patch

- Actual line/branch coverage percentage. Add gcov/llvm-cov later if you want numerical gates.
- Real benchmark baselines. The harness is included, but the repo needs committed or release-attached baseline results after running on known hardware.
- Full clang-tidy enforcement. A config is included, but CI does not force it yet because the current code should be cleaned under a separate commit before making it blocking.

## Suggested commit message

```text
chore: add waivm quality gates and benchmark harness
```
