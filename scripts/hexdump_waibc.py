#!/usr/bin/env python3
"""Small inspector for waivm .waibc files."""
import argparse
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="dump a .waibc file in hex")
    parser.add_argument("file")
    args = parser.parse_args()

    data = Path(args.file).read_bytes()
    for offset in range(0, len(data), 16):
        chunk = data[offset:offset + 16]
        hexed = " ".join(f"{b:02x}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"{offset:08x}  {hexed:<47}  {ascii_part}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
