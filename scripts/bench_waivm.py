#!/usr/bin/env python3
"""Small benchmark harness for waivm CLI commands.

The harness measures process-level latency for representative programs. It is not
a substitute for instruction-level profiling, but it gives repeatable before/after
numbers for parser, verifier, trace, and run regressions.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import statistics
import subprocess
import sys
import time
from collections.abc import Sequence
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class SampleSet:
    command: list[str]
    samples_ms: list[float]

    @property
    def mean_ms(self) -> float:
        return statistics.fmean(self.samples_ms)

    @property
    def median_ms(self) -> float:
        return statistics.median(self.samples_ms)

    @property
    def p95_ms(self) -> float:
        return percentile(self.samples_ms, 95)

    @property
    def stdev_ms(self) -> float:
        if len(self.samples_ms) < 2:
            return 0.0
        return statistics.stdev(self.samples_ms)

    def to_json(self) -> dict[str, object]:
        return {
            "command": self.command,
            "iterations": len(self.samples_ms),
            "mean_ms": round(self.mean_ms, 4),
            "median_ms": round(self.median_ms, 4),
            "p95_ms": round(self.p95_ms, 4),
            "min_ms": round(min(self.samples_ms), 4),
            "max_ms": round(max(self.samples_ms), 4),
            "stdev_ms": round(self.stdev_ms, 4),
            "samples_ms": [round(value, 4) for value in self.samples_ms],
        }


def percentile(values: Sequence[float], percentile_value: float) -> float:
    if not values:
        raise ValueError("cannot compute percentile of an empty list")
    ordered = sorted(values)
    if len(ordered) == 1:
        return ordered[0]
    rank = (len(ordered) - 1) * percentile_value / 100.0
    lower = int(rank)
    upper = min(lower + 1, len(ordered) - 1)
    weight = rank - lower
    return ordered[lower] * (1.0 - weight) + ordered[upper] * weight


def run_once(command: Sequence[str], *, timeout: float) -> float:
    started = time.perf_counter_ns()
    completed = subprocess.run(
        command,
        cwd=ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        timeout=timeout,
        check=False,
    )
    elapsed_ms = (time.perf_counter_ns() - started) / 1_000_000.0
    if completed.returncode != 0:
        stderr = completed.stderr.decode("utf-8", errors="replace")[-1200:]
        raise RuntimeError(f"command failed with exit {completed.returncode}: {' '.join(command)}\n{stderr}")
    return elapsed_ms


def measure(command: list[str], *, warmup: int, iterations: int, timeout: float) -> SampleSet:
    for _ in range(warmup):
        run_once(command, timeout=timeout)
    samples = [run_once(command, timeout=timeout) for _ in range(iterations)]
    return SampleSet(command=command, samples_ms=samples)


def load_baseline(path: pathlib.Path) -> dict[str, float]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise ValueError("baseline must be a JSON object")
    out: dict[str, float] = {}
    for key in ("mean_ms", "median_ms", "p95_ms"):
        value = data.get(key)
        if isinstance(value, int | float):
            out[key] = float(value)
    if not out:
        raise ValueError("baseline must contain mean_ms, median_ms, or p95_ms")
    return out


def check_regression(result: SampleSet, baseline_path: pathlib.Path, max_regression: float) -> None:
    baseline = load_baseline(baseline_path)
    current = result.to_json()
    failures: list[str] = []
    for key, before in baseline.items():
        after = float(current[key])
        allowed = before * (1.0 + max_regression)
        if after > allowed:
            failures.append(f"{key}: {after:.4f}ms > allowed {allowed:.4f}ms from baseline {before:.4f}ms")
    if failures:
        joined = "\n".join(failures)
        raise RuntimeError(f"benchmark regression beyond {max_regression:.1%}:\n{joined}")


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Benchmark waivm CLI latency.")
    parser.add_argument("--waivm", default="build/waivm", help="path to waivm executable")
    parser.add_argument("--mode", default="run", choices=("run", "verify", "trace", "dis"))
    parser.add_argument("--program", default="examples/factorial.wai", help="program or bytecode file to run")
    parser.add_argument("--iterations", type=int, default=30)
    parser.add_argument("--warmup", type=int, default=5)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--output", help="write JSON result to this file")
    parser.add_argument("--baseline", help="compare against a previous JSON result")
    parser.add_argument("--max-regression", type=float, default=0.05, help="allowed fractional regression")
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    if args.iterations < 1:
        print("--iterations must be at least 1", file=sys.stderr)
        return 2
    if args.warmup < 0:
        print("--warmup cannot be negative", file=sys.stderr)
        return 2

    waivm = pathlib.Path(args.waivm)
    if not waivm.is_absolute():
        waivm = ROOT / waivm
    if not waivm.exists():
        print(f"waivm executable not found: {waivm}", file=sys.stderr)
        return 2

    program = pathlib.Path(args.program)
    if not program.is_absolute():
        program = ROOT / program
    if not program.exists():
        print(f"program not found: {program}", file=sys.stderr)
        return 2

    command = [str(waivm), args.mode, str(program)]
    try:
        result = measure(command, warmup=args.warmup, iterations=args.iterations, timeout=args.timeout)
        payload = result.to_json()
        rendered = json.dumps(payload, indent=2, sort_keys=True)
        print(rendered)
        if args.output:
            pathlib.Path(args.output).write_text(rendered + "\n", encoding="utf-8")
        if args.baseline:
            check_regression(result, pathlib.Path(args.baseline), args.max_regression)
    except (RuntimeError, subprocess.TimeoutExpired, ValueError) as exc:
        print(f"benchmark failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
