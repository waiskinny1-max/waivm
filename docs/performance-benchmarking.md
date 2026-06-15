# waivm benchmarking notes

`waivm` has several different performance surfaces. Do not flatten them into one vague claim like "fast". Measure the path that changed.

## Paths worth measuring

| Path | Command shape | What it measures |
|---|---|---|
| Source execution | `waivm run examples/factorial.wai` | parse + assemble + execute overhead |
| Bytecode execution | `waivm run program.waibc` | loader + VM dispatch overhead |
| Verification | `waivm verify program.wai` | parser/assembler + verifier cost |
| Trace | `waivm trace program.wai` | stepping/reporting cost |
| Disassembly | `waivm dis program.waibc` | decode + text rendering cost |

## Baseline procedure

1. Build once in the mode you care about.
2. Run at least five warmup iterations.
3. Run at least thirty measured iterations.
4. compare p95, mean, and max; do not use averages alone.
5. If p95 regresses, profile before rewriting.

Example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
python3 scripts/bench_waivm.py \
  --waivm build/waivm \
  --mode run \
  --program examples/factorial.wai \
  --iterations 50 \
  --warmup 8 \
  --output .bench-factorial-release.json
```

Regression check:

```bash
python3 scripts/bench_waivm.py \
  --waivm build/waivm \
  --mode run \
  --program examples/factorial.wai \
  --baseline .bench-factorial-release.json \
  --max-regression 0.05
```

## Interpretation rules

- Process-level latency includes program startup. It is still useful for CLI user experience.
- Bytecode execution benchmarks should use `.waibc` inputs when isolating VM dispatch.
- Source execution benchmarks are useful when changing assembler/parser code.
- Trace and debugger improvements need separate measurements because text rendering can dominate runtime.
- Do not claim a speedup unless the method, build mode, CPU, input, iterations, and before/after numbers are recorded.
