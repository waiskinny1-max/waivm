# waivm code-quality policy

This policy turns the repo's quality expectations into gates. It is intentionally practical: a contributor should be able to run the same checks locally that CI runs on a pull request.

## Scope

Applies to:

- C17 VM, assembler, disassembler, verifier, debugger, trace mode, and CLI code.
- NASM Linux x86-64 execution core.
- Rust terminal UI under `tui/`.
- Python and shell scripts under `scripts/`.
- Documentation that states current behavior, commands, formats, or limitations.

## Non-negotiable rules

1. **Correctness before speed.** No performance rewrite is accepted without a correctness test that would fail on the old bug or guard the changed path.
2. **No silent weakening of the VM contract.** Changes to bytecode encoding, instruction semantics, register behavior, memory bounds, stack behavior, or verifier rules require tests and documentation updates.
3. **No swallowed failures.** Errors must keep enough context to identify the failing file, instruction, byte offset, register, or command.
4. **No fake coverage claims.** README and docs may describe only implemented behavior.
5. **No unbounded parsing assumptions.** Inputs from `.wai` and `.waibc` files are untrusted and must be bounds-checked before use.
6. **No convenience layer that only hides control flow.** Abstractions must reduce duplication, isolate a boundary, or make an invariant explicit.

## Required gates

Every merge should pass:

```bash
python3 scripts/quality_gate.py --full
```

CI separately runs:

- Debug C/ASM build.
- Release C/ASM build.
- `ctest --output-on-failure`.
- ASan/UBSan build for native tests.
- Python syntax checks for scripts.
- Shell syntax checks for shell scripts.
- Rust `fmt`, `check`, and `clippy -D warnings` for `tui/` when present.

## Review thresholds

These are review gates, not mechanical dogma. A waiver must name the owner, reason, and expiry condition.

| Area | Default threshold | Waiver trigger |
|---|---:|---|
| Function size | Prefer under 90 logical lines | Parsing/state-machine functions may exceed this only with tests around each branch family |
| Nesting depth | Prefer `<= 4` | Split validation, decoding, and execution paths if nesting grows |
| Cyclomatic complexity | Prefer `<= 10` for new/changed functions | Add a short note in review explaining why a table-driven rewrite is worse |
| Test coverage | Core changed behavior must have test coverage | Do not merge parser/verifier/runtime changes without direct tests |
| Benchmark regression | Keep representative command p95 within 5% after baseline exists | Waive only after profiling shows a deliberate trade-off |

## Required tests by change type

| Change touches | Minimum tests |
|---|---|
| Assembler parsing | Valid input, malformed input, label resolution, numeric boundary |
| Bytecode writer/loader | Round trip, bad magic/version, truncated header/body, oversized count |
| VM runtime | Direct instruction behavior, memory bounds, stack behavior, halt/error path |
| Verifier | Accept valid bytecode, reject invalid jump/register/memory/stack patterns |
| Debugger | Command parsing, stepping, breakpoint behavior, quit/error handling |
| Trace/disassembler | Stable text for representative programs and malformed input behavior |
| Rust TUI | Command construction, file discovery, process-error rendering, keyboard command mapping |
| Scripts | Argument validation, missing file/tool behavior, non-zero subprocess behavior |

## Performance workflow

Create a baseline after a clean build:

```bash
python3 scripts/bench_waivm.py --waivm build/waivm --mode run --program examples/factorial.wai --output .bench-factorial.json
```

Compare a future change:

```bash
python3 scripts/bench_waivm.py --waivm build/waivm --mode run --program examples/factorial.wai --baseline .bench-factorial.json
```

Use this for regressions in parser, verifier, trace, and runtime paths. Do not treat a single process-level number as proof of instruction-level speed; use it as an early warning, then profile.

## Documentation standard

Docs must separate current behavior from future work. When changing behavior, update the closest document:

- `README.md` for user-facing commands.
- `docs/bytecode-format.md` for binary format changes.
- `docs/verifier.md` for verifier behavior.
- `docs/debugger.md` for debugger UX.
- `docs/terminal-ui.md` and `tui/README.md` for TUI behavior.
- `docs/roadmap.md` for planned work only.

## Waiver format

Use this in a pull request when a gate is intentionally bypassed:

```md
### Quality waiver

- Gate waived:
- Reason:
- Risk:
- Compensating test/review:
- Owner:
- Expiry condition:
```
