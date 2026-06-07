#!/usr/bin/env python3
"""Golden-output tests for waivm example programs."""
from __future__ import annotations

import pathlib
import subprocess
import sys


def run_case(exe: pathlib.Path, repo: pathlib.Path, name: str) -> int:
    source = repo / "examples" / f"{name}.wai"
    expected_path = repo / "tests" / "golden" / f"{name}.out"
    expected = expected_path.read_text(encoding="utf-8")
    proc = subprocess.run(
        [str(exe), "run", str(source)],
        cwd=str(repo),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        sys.stderr.write(f"{name}: waivm exited {proc.returncode}\n{proc.stderr}\n")
        return 1
    if proc.stdout != expected:
        sys.stderr.write(f"{name}: output mismatch\n")
        sys.stderr.write(f"expected:\n{expected!r}\nactual:\n{proc.stdout!r}\n")
        return 1
    return 0


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        sys.stderr.write("usage: run_golden.py <waivm-exe> <repo-root>\n")
        return 2
    exe = pathlib.Path(argv[1]).resolve()
    repo = pathlib.Path(argv[2]).resolve()
    cases = [
        "sum",
        "factorial",
        "fib",
        "loop",
        "branch",
        "memory",
        "call",
        "compare",
        "bitwise",
        "modulo",
    ]
    failures = sum(run_case(exe, repo, case) for case in cases)
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
