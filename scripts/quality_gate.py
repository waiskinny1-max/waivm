#!/usr/bin/env python3
"""Local quality gate for waivm.

This script intentionally uses only the Python standard library. It is meant to be
safe to run before a commit and close to what CI enforces.
"""

from __future__ import annotations

import argparse
import compileall
import os
import pathlib
import shutil
import subprocess
import sys
from collections.abc import Sequence

ROOT = pathlib.Path(__file__).resolve().parents[1]


class GateError(RuntimeError):
    """Raised when a required quality gate fails."""


def has_command(name: str) -> bool:
    return shutil.which(name) is not None


def run(cmd: Sequence[str], *, cwd: pathlib.Path = ROOT, env: dict[str, str] | None = None) -> None:
    printable = " ".join(cmd)
    print(f"\n$ {printable}", flush=True)
    completed = subprocess.run(cmd, cwd=cwd, env=env, check=False)
    if completed.returncode != 0:
        raise GateError(f"command failed with exit {completed.returncode}: {printable}")


def check_python_scripts() -> None:
    scripts_dir = ROOT / "scripts"
    if not scripts_dir.exists():
        return
    print("\n[python] compiling scripts", flush=True)
    ok = compileall.compile_dir(str(scripts_dir), quiet=1, force=True)
    if not ok:
        raise GateError("python script syntax check failed")


def check_shell_scripts() -> None:
    scripts = sorted((ROOT / "scripts").glob("*.sh"))
    if not scripts:
        return
    if not has_command("bash"):
        raise GateError("bash is required to syntax-check shell scripts")
    for script in scripts:
        run(["bash", "-n", str(script)])


def check_trailing_whitespace() -> None:
    ignored_parts = {".git", "build", "target", "__pycache__"}
    suffixes = {".c", ".h", ".asm", ".rs", ".toml", ".md", ".py", ".sh", ".yml", ".yaml", ".txt"}
    offenders: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix not in suffixes:
            continue
        if ignored_parts.intersection(path.parts):
            continue
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            continue
        for number, line in enumerate(lines, start=1):
            if line.rstrip() != line:
                offenders.append(f"{path.relative_to(ROOT)}:{number}")
    if offenders:
        joined = "\n".join(offenders[:80])
        extra = "" if len(offenders) <= 80 else f"\n... and {len(offenders) - 80} more"
        raise GateError(f"trailing whitespace found:\n{joined}{extra}")


def cmake_generator() -> list[str]:
    if has_command("ninja"):
        return ["-G", "Ninja"]
    return []


def configure_and_test(build_dir: pathlib.Path, build_type: str, *, sanitizers: bool) -> None:
    if not has_command("cmake"):
        raise GateError("cmake is required for the native quality gate")
    if not has_command("nasm"):
        raise GateError("nasm is required for waivm's assembly runtime")

    cmd = ["cmake", "-S", ".", "-B", str(build_dir), *cmake_generator(), f"-DCMAKE_BUILD_TYPE={build_type}"]
    if sanitizers:
        cmd.extend(
            [
                "-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
            ]
        )
    run(cmd)
    run(["cmake", "--build", str(build_dir), "--parallel"])

    env = os.environ.copy()
    if sanitizers:
        env.setdefault("ASAN_OPTIONS", "detect_leaks=1:halt_on_error=1")
        env.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
    run(["ctest", "--test-dir", str(build_dir), "--output-on-failure"], env=env)


def cargo_manifest() -> pathlib.Path:
    return ROOT / "tui" / "Cargo.toml"


def check_tui(*, full: bool) -> None:
    manifest = cargo_manifest()
    if not manifest.exists():
        return
    if not has_command("cargo"):
        raise GateError("cargo is required because tui/Cargo.toml exists")

    if has_command("rustfmt"):
        run(["cargo", "fmt", "--manifest-path", str(manifest), "--", "--check"])
    else:
        print("\n[rust] rustfmt not found; skipping format check", flush=True)

    run(["cargo", "check", "--manifest-path", str(manifest)])

    if full and has_command("cargo-clippy"):
        run(["cargo", "clippy", "--manifest-path", str(manifest), "--", "-D", "warnings"])
    elif full:
        print("\n[rust] clippy not found; cargo check already passed", flush=True)


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run waivm local quality gates.")
    parser.add_argument("--build-dir", default="build/quality", help="CMake build directory")
    parser.add_argument("--build-type", default="Debug", choices=("Debug", "Release", "RelWithDebInfo"))
    parser.add_argument("--fast", action="store_true", help="Only run syntax and whitespace checks")
    parser.add_argument("--skip-native", action="store_true", help="Skip CMake build and ctest")
    parser.add_argument("--skip-tui", action="store_true", help="Skip Rust terminal UI checks")
    parser.add_argument("--sanitizers", action="store_true", help="Build C code with ASan/UBSan")
    parser.add_argument("--full", action="store_true", help="Run stricter optional checks where tools exist")
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    try:
        check_python_scripts()
        check_shell_scripts()
        check_trailing_whitespace()
        if not args.fast and not args.skip_native:
            configure_and_test(ROOT / args.build_dir, args.build_type, sanitizers=args.sanitizers)
        if not args.fast and not args.skip_tui:
            check_tui(full=args.full)
    except GateError as exc:
        print(f"\nquality gate failed: {exc}", file=sys.stderr)
        return 1
    print("\nquality gate passed", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
